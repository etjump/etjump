#include "cg_local.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "etj_awaited_command_handler.h"
#include "etj_client_commands_handler.h"
#include "../game/etj_string_utilities.h"
#include "etj_inline_command_parser.h"
#include "etj_player_events_handler.h"


ETJump::AwaitedCommandHandler::AwaitedCommandHandler(
    std::shared_ptr<ClientCommandsHandler> consoleCommandsHandler,
    std::function<void(const char*)> sendConsoleCommand, 
    std::function<void(const char*)> printToConsole
)
    : _consoleCommandsHandler(consoleCommandsHandler),
    _sendConsoleCommand(sendConsoleCommand),
    _printToConsole(printToConsole)
{
    _awaitedCommands.clear();
    _consoleCommandsHandler->subscribe("await", [this](const std::vector<std::string>& args)
    {
		if(cgs.clientinfo->timerunActive)
		{
			message("^3Error: ^7cannot use ^3await ^7during timeruns.");
			return;
		}
        this->awaitCommand(args);
    });

	playerEventsHandler->subscribe("timerun:start", [&](const std::vector<std::string>& args)
	{
		if (_awaitedCommands.size())
		{
			CG_AddPMItem(PM_MESSAGE, "^7Timerun started, ^3await ^7queue cleared.", cgs.media.stopwatchIcon);
			_awaitedCommands.clear();
		}
	});
}

ETJump::AwaitedCommandHandler::~AwaitedCommandHandler()
{
    _consoleCommandsHandler->unsubcribe("await");
}

void ETJump::AwaitedCommandHandler::runFrame()
{
    std::vector<AwaitedCommand*> executedCommands;
    for (const auto& awaitedCommand : _awaitedCommands)
    {
        awaitedCommand->currentFrameCount++;

        if (awaitedCommand->currentFrameCount >= awaitedCommand->requiredFrameCount)
        {
            executedCommands.push_back(awaitedCommand.get());

            for (const auto& command : awaitedCommand->commands)
            {
                executeConsoleCommand(command);
            }
        }
    }

    for (auto executedCommand : executedCommands)
    {
        _awaitedCommands.erase(std::remove_if(_awaitedCommands.begin(), _awaitedCommands.end(), [&](const std::unique_ptr<AwaitedCommand>& command)
        {
            return command.get() == executedCommand;
        }));
    }
}

void ETJump::AwaitedCommandHandler::message(const std::string& message)
{
    _printToConsole(stringFormat("%s\n", message).c_str());
}

void ETJump::AwaitedCommandHandler::executeConsoleCommand(const std::string& command)
{
    _sendConsoleCommand(stringFormat("%s\n", command).c_str());
}

void ETJump::AwaitedCommandHandler::awaitCommand(const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        message("^3Usage: ^7await <number of frames> <command> | <another command> | ...\nSchedules a set of commands to be executed later without interrupting other user actions.");
        return;
    }

    auto waitedFramesStr = args[0];
    int waitedFrames = 0;
    std::string outOfRangeError = stringFormat("^3Error: ^7First parameter (number of frames) must be between ^31 ^7and ^3%s^7.", std::numeric_limits<int>::max());
    try
    {
        waitedFrames = std::stoi(waitedFramesStr);
    } catch (const std::out_of_range&)
    {
        message(outOfRangeError);
        return;
    } catch (const std::invalid_argument&)
    {
        message("^3Error: ^7First parameter (number of frames) must be a valid integer.");
        return;
    }

    if (waitedFrames < 1)
    {
        message(outOfRangeError);
        return;
    }

    auto command = std::unique_ptr<AwaitedCommand>(new AwaitedCommand());
    command->currentFrameCount = 0;
    command->requiredFrameCount = waitedFrames;
    command->commands = InlineCommandParser().parse(std::vector<std::string>(args.begin() + 1, args.end()));

	auto numCommandsString = command->commands.size() == 1 ? "command" : "commands";
	auto numFramesString = command->requiredFrameCount == 1 ? "frame" : "frames";
    message(stringFormat("Executing ^3%d ^7%s after ^3%d ^7%s.", command->commands.size(), numCommandsString, command->requiredFrameCount, numFramesString));
    _awaitedCommands.push_back(std::move(command));
}
