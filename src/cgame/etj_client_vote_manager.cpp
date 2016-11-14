#include "etj_client_vote_manager.h"
#include "etj_client_commands_handler.h"
#include "etj_client_printer.h"
#include "cg_local.h"

void printVoteUsage()
{
	ETJump::ClientPrinter::writeLine("^3usage: ^7/vote yes|no");
}

ETJump::ClientVoteManager::ClientVoteManager(std::shared_ptr<IClientCommandsHandler> clientCommandsHandler, std::shared_ptr<IClientCommandsHandler> serverCommandsHandler, std::shared_ptr<IConfigStringEventsHandler> configStringEventsHandler):
	_clientCommandsHandler(clientCommandsHandler),
	_serverCommandsHandler(serverCommandsHandler),
	_configStringEventsHandler(configStringEventsHandler)
{
	if (_clientCommandsHandler == nullptr)
	{
		CG_Error("ClientVoteManager: clientCommandsHandler is null\n");
		return;
	}

	clientCommandsHandler->subscribe("vote", [&](const std::vector<std::string>& args)
		{
			if (args.size() == 0)
			{
				printVoteUsage();
				return;
			}

			auto voteParam = VoteParameter::No;
			if (args[0] == "yes")
			{
				voteParam = VoteParameter::Yes;
			}
			else if (args[0] == "no")
			{
				voteParam = VoteParameter::No;
			}
			else
			{
				printVoteUsage();
				return;
			}

			auto message = handleVote(voteParam);
			if (message.length() > 0)
			{
				ClientPrinter::writeLine(message);
			}
		});

	_eventHandlerId = _configStringEventsHandler->subscribe([&](int configString, const std::string& value)
	{
		this->configStringModified(configString, value);
	});
}

ETJump::ClientVoteManager::~ClientVoteManager()
{
}

std::string ETJump::ClientVoteManager::handleVote(VoteParameter voteParameter)
{
	return "";
}

void ETJump::ClientVoteManager::configStringModified(int configString, const std::string value)
{
	CG_Printf("%d: %s\n", configString, value.c_str());
}
