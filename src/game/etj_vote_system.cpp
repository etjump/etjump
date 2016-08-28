#include "etj_vote_system.h"
#include "etj_server_commands_handler.h"
#include <stdexcept>
#include "etj_printer.h"
#include <boost/algorithm/string.hpp>


ETJump::VoteSystem::VoteSystem(ServerCommandsHandler* commandsHandler): _commandsHandler(commandsHandler), _voteQueue{}, _currentVote(nullptr), _levelTime(0)
{
	if (_commandsHandler == nullptr)
	{
		throw std::runtime_error("commandsHandler is undefined.");
	}

	initCommands();
}

ETJump::VoteSystem::~VoteSystem()
{
	_currentVote = nullptr;
	// empty the queue on map change
	_voteQueue = std::queue<std::unique_ptr<Vote>>{};
}

void ETJump::VoteSystem::runFrame(int levelTime)
{
	_levelTime = levelTime;
}

void ETJump::VoteSystem::initCommands()
{
	if (!_commandsHandler->subscribe("vote", [&](int clientNum, const std::vector<std::string>& args)
	{
		vote(clientNum, args);
	}))
	{
		throw std::runtime_error("vote command has already been subscribed to.");
	}

	if (!_commandsHandler->subscribe("callvote", [&](int clientNum, const std::vector<std::string>& args)
	{
		callVote(clientNum, args);
	}))
	{
		throw std::runtime_error("callvote command has already been subscribed to.");
	}
}

void ETJump::VoteSystem::vote(int clientNum, const std::vector<std::string>& args)
{
	const auto usage = "^3usage: ^7/vote <yes|no>";

	if (args.size() == 1)
	{
		Printer::SendConsoleMessage(clientNum, usage);
		return;
	}

	auto result = parseVoteArgs(args);
	if (!result.success)
	{
		Printer::SendConsoleMessage(clientNum, usage);
		return;
	}
}

ETJump::VoteSystem::VoteParseResult ETJump::VoteSystem::parseVoteArgs(const std::vector<std::string>& args)
{
	VoteParseResult result{};
	result.voted = Voted::No;
	char lwr = tolower(args[1][0]);
	if (lwr == 'y' || lwr == '1')
	{
		result.voted = Voted::Yes;
	} else if (lwr == 'n' || lwr == '0')
	{
		result.voted = Voted::No;
	} else
	{
		result.success = false;
	}
	result.success = true;
	return result;
}

void ETJump::VoteSystem::callVote(int clientNum, const std::vector<std::string>& args)
{
	auto vote = boost::to_lower_copy(args[1]);
	if (vote == "map")
	{
		mapVote(clientNum, args);
	} else if (vote == "randommap")
	{
		createSimpleVote(clientNum, VoteType::RandomMap);
	} else if (vote == "maprestart")
	{
		createSimpleVote(clientNum, VoteType::MapRestart);
	} else
	{
		Printer::SendConsoleMessage(clientNum, (boost::format("^3error: ^7unknown vote type: %s.") % args[1]).str());
		return;
	}
}

void ETJump::VoteSystem::mapVote(int clientNum, const std::vector<std::string>& args)
{
	std::unique_ptr<Vote> vote(new Vote());
	vote->type = VoteType::Map;

	if (args.size() == 1)
	{
		
	}
}

void ETJump::VoteSystem::createSimpleVote(int clientNum, VoteType type)
{
	std::unique_ptr<Vote> vote(new Vote());
	vote->type = type;
	if (callOrQueueVote(move(vote)) == QueuedVote::Yes)
	{
		Printer::SendConsoleMessage(clientNum, (boost::format("^gVote: ^7Vote is currently in progress. Your vote has been added to the vote queue. There are currently %d votes in the queue.\n") % _voteQueue.size()).str());
	} 
}

ETJump::VoteSystem::QueuedVote ETJump::VoteSystem::callOrQueueVote(std::unique_ptr<Vote> vote)
{
	if (_currentVote == nullptr)
	{
		vote->startTime = _levelTime;
		_currentVote = move(vote);
		return QueuedVote::No;
	}
	else
	{
		_voteQueue.push(move(vote));
		return QueuedVote::Yes;
	}
}
