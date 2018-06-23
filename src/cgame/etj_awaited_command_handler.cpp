#include "etj_awaited_command_handler.h"
#include "etj_client_commands_handler.h"
#include "../game/etj_string_utilities.h"
#include "etj_inline_command_parser.h"


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
        this->awaitCommand(args);
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
        message("usage: await <number of frames> <command> | <another command> | ...");
        return;
    }

    auto waitedFramesStr = args[0];
    int waitedFrames = 0;
    std::string outOfRangeError = stringFormat("first parameter (number of frames) must be between 1 and %s", std::numeric_limits<int>::max());
    try
    {
        waitedFrames = std::stoi(waitedFramesStr);
    } catch (const std::out_of_range&)
    {
        message(outOfRangeError);
        return;
    } catch (const std::invalid_argument&)
    {
        message("first parameter (number of frames) must be a valid integer");
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

    message(stringFormat("Executing %d commands in %d frames", command->commands.size(), command->requiredFrameCount));
    _awaitedCommands.push_back(std::move(command));
}
