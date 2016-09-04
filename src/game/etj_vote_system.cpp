#include "etj_vote_system.h"
#include "etj_imap_queries.h"
#include "etj_server_commands_handler.h"
#include <stdexcept>
#include "etj_printer.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "etj_log.h"


std::string ETJump::VoteSystem::Vote::toString()
{
	switch (type)
	{
	case VoteType::MapRestart: return "Restart map";
	case VoteType::Map: return (boost::format("Change map to: %s") % reinterpret_cast<MapVote*>(this)->map).str();
	case VoteType::RandomMap: return "Random map";
	default: 
		Log::error("Unknown vote type.");
		return "";
	}
}

ETJump::VoteSystem::VoteSystem(ServerCommandsHandler* commandsHandler, IMapQueries* mapQueries):
	_commandsHandler(commandsHandler),
	_mapQueries(mapQueries),
	_voteQueue{},
	_currentVote(nullptr),
	_levelTime(0)
{
	if (_commandsHandler == nullptr)
	{
		Log::error("commandsHandler is null.");
		return;
	}

	if (_mapQueries == nullptr)
	{
		Log::error("mapQueries is null.");
		return;
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
		Log::error("vote command has already been subscribed to.");
		return;
	}

	if (!_commandsHandler->subscribe("callvote", [&](int clientNum, const std::vector<std::string>& args)
	                                 {
		                                 callVote(clientNum, args);
	                                 }))
	{
		Log::error("callvote command has already been subscribed to.");
		return;
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
	}
	else if (lwr == 'n' || lwr == '0')
	{
		result.voted = Voted::No;
	}
	else
	{
		result.success = false;
	}
	result.success = true;
	return result;
}

void ETJump::VoteSystem::callVote(int clientNum, const std::vector<std::string>& args)
{
	if (args.size() == 1)
	{
		Printer::SendConsoleMessage(clientNum, "^3usage: ^7/callvote <vote> <additional parameters>");
		return;
	}

	auto type = boost::to_lower_copy(args[1]);
	if (type == "map")
	{
		mapVote(clientNum, args);
	}
	else if (type == "randommap")
	{
		createSimpleVote(clientNum, VoteType::RandomMap);
	}
	else if (type == "maprestart")
	{
		createSimpleVote(clientNum, VoteType::MapRestart);
	}
	else
	{
		Printer::SendConsoleMessage(clientNum, (boost::format("^3error: ^7unknown vote type: %s.") % args[1]).str());
		return;
	}
}

void ETJump::VoteSystem::displayVoteQueueResult(int clientNum, QueuedVote voteWasQueued)
{
	if (voteWasQueued == QueuedVote::Yes)
	{
		Printer::SendConsoleMessage(clientNum, boost::format("^gvote:^7 vote is currently in progress. Your vote has been added to the vote queue. There are currently %d votes in the queue.") % _voteQueue.size());
	} else
	{
		Printer::BroadcastConsoleMessage(boost::format("^7%s ^7called a vote: %s") % std::to_string(clientNum) % _currentVote.get()->toString());
	}
}

void ETJump::VoteSystem::mapVote(int clientNum, const std::vector<std::string>& args)
{
	std::unique_ptr<Vote> vote(new MapVote());
	vote->type = VoteType::Map;

	if (args.size() != 3)
	{
		Printer::SendConsoleMessage(clientNum, "^3usage: /callvote map <map name>");
		return;
	}

	auto map = boost::to_lower_copy(args[2]);
	auto matches = _mapQueries->matches(map);
	if (matches.size() == 0)
	{
		Printer::SendConsoleMessage(clientNum, (boost::format("^1error: ^7could not find a matching map with name: \"%s\"") % map).str());
		return;
	}

	if (matches.size() > 1)
	{
		Printer::SendConsoleMessage(clientNum, (boost::format("^1error: ^7found multiple matching maps:\n* %s") % boost::join(matches, "\n* ")).str());
		return;
	}

	reinterpret_cast<MapVote*>(vote.get())->map = matches[0];

	displayVoteQueueResult(clientNum, callOrQueueVote(move(vote)));
}

void ETJump::VoteSystem::createSimpleVote(int clientNum, VoteType type)
{
	std::unique_ptr<Vote> vote(new Vote());
	vote->type = type;
	displayVoteQueueResult(clientNum, callOrQueueVote(move(vote)));
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
