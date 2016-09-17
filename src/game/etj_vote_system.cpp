#include "etj_vote_system.h"
#include "etj_imap_queries.h"
#include "etj_server_commands_handler.h"
#include "etj_printer.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "etj_log.h"
#include "etj_event_aggregator.h"

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

ETJump::VoteSystem::VoteSystem(ServerCommandsHandler* commandsHandler, EventAggregator* eventAggregator, IMapQueries* mapQueries, VoteSystemOptions options):

	_commandsHandler(commandsHandler),
	_mapQueries(mapQueries),
	_eventAggregator(eventAggregator),
	_eventHandles{},
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

	if (_eventAggregator == nullptr)
	{
		Log::error("eventAggregator is null.");
		return;
	}

	_options = std::move(options);

	initCommands();
	initEventListening();
	initVoters();
}

ETJump::VoteSystem::~VoteSystem()
{
	_currentVote = nullptr;
	// empty the queue on map change
	_voteQueue = std::queue<std::unique_ptr<Vote>>{};
	for (const auto & h : _eventHandles)
	{
		_eventAggregator->unsubcribe(h);
	}
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

void ETJump::VoteSystem::initEventListening()
{
	_eventHandles.push_back(_eventAggregator->subscribe(EventAggregator::ServerEventType::RunFrame, [&](const EventAggregator::Payload *payload)
	{
		_levelTime = payload->integers[0];

		// just pop the current vote if it has not passed yet
		if (_currentVote && _levelTime > _currentVote->startTime + _options.voteDurationMs)
		{
			auto numNo = 0;
			auto numYes = 0;
			for (const auto & voter : _voters)
			{
				if (voter.status == VoteStatus::No) ++numNo;
				if (voter.status == VoteStatus::Yes) ++numYes;
			}

			Printer::BroadCastBannerMessage(boost::format("^3Voted failed. %d/%d. Required %d/%d.") % numYes % _voters.size() % (_voters.size() * 0.5) % _voters.size());
			_currentVote = popNextVote();
			if (_currentVote)
			{
			}
		}
	}));
}

void ETJump::VoteSystem::initVoters()
{
	for (auto & v : _voters)
	{
		v.numRevotes = 0;
		v.status = VoteStatus::NotYetVoted;
	}
}

void ETJump::VoteSystem::vote(int clientNum, const std::vector<std::string>& args)
{
	const auto usage = "^3usage: ^7/vote <yes|no>\n";

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

	if (_voters[clientNum].status == VoteStatus::NotYetVoted)
	{
		_voters[clientNum].status = result.voted;
		Printer::SendConsoleMessage(clientNum, boost::format("Voted %s\n") % (result.voted == VoteStatus::Yes ? "yes" : "no"));
		return;
	}

	if (_voters[clientNum].numRevotes >= _options.maxRevotes || _voters[clientNum].status == result.voted)
	{
		Printer::SendConsoleMessage(clientNum, boost::format("^1Error: ^7You've already revoted %d times. You cannot revote again.\n") % _options.maxRevotes);
		return;
	}

	_voters[clientNum].status = result.voted;
	Printer::SendConsoleMessage(clientNum, boost::format("Changed vote from %s to %s\n") 
		% (result.voted == VoteStatus::Yes ? "no" : "yes") 
			% (result.voted == VoteStatus::Yes ? "yes" : "no"));
	_voters[clientNum].numRevotes++;
}

ETJump::VoteSystem::VoteParseResult ETJump::VoteSystem::parseVoteArgs(const std::vector<std::string>& args)
{
	VoteParseResult result{};
	result.voted = VoteStatus::No;
	char lwr = tolower(args[1][0]);
	if (lwr == 'y' || lwr == '1')
	{
		result.voted = VoteStatus::Yes;
	}
	else if (lwr == 'n' || lwr == '0')
	{
		result.voted = VoteStatus::No;
	}
	else
	{
		result.success = false;
		result.voted = VoteStatus::NotYetVoted;
		return result;
	}
	result.success = true;
	return result;
}

void ETJump::VoteSystem::callVote(int clientNum, const std::vector<std::string>& args)
{
	if (args.size() == 1)
	{
		Printer::SendConsoleMessage(clientNum, "^3usage: ^7/callvote <vote> <additional parameters>\n");
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
		Printer::SendConsoleMessage(clientNum, (boost::format("^3error: ^7unknown vote type: %s.\n") % args[1]).str());
		return;
	}
}

void ETJump::VoteSystem::displayVoteQueueResult(int clientNum, QueuedVote voteWasQueued)
{
	if (voteWasQueued == QueuedVote::Yes)
	{
		Printer::SendConsoleMessage(clientNum, boost::format("^gvote:^7 vote is currently in progress. Your vote has been added to the vote queue. There are currently %d votes in the queue.\n") % _voteQueue.size());
	} else
	{
		Printer::BroadcastConsoleMessage(boost::format("^7%s ^7called a vote: %s\n") % std::to_string(clientNum) % _currentVote.get()->toString());
	}
}

void ETJump::VoteSystem::mapVote(int clientNum, const std::vector<std::string>& args)
{
	std::unique_ptr<Vote> vote(new MapVote());
	vote->type = VoteType::Map;

	if (args.size() != 3)
	{
		Printer::SendConsoleMessage(clientNum, "^3usage: /callvote map <map name>\n");
		return;
	}

	auto map = boost::to_lower_copy(args[2]);
	auto matches = _mapQueries->matches(map);
	if (matches.size() == 0)
	{
		Printer::SendConsoleMessage(clientNum, (boost::format("^1error: ^7could not find a matching map with name: \"%s\"\n") % map).str());
		return;
	}

	if (matches.size() > 1)
	{
		Printer::SendConsoleMessage(clientNum, (boost::format("^1error: ^7found multiple matching maps:\n* %s\n") % boost::join(matches, "\n* ")).str());
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
	_voteQueue.push(move(vote));
	return QueuedVote::Yes;
}

std::unique_ptr<ETJump::VoteSystem::Vote> ETJump::VoteSystem::popNextVote()
{
	if (_voteQueue.size() > 0)
	{
		auto vote = move(_voteQueue.front());
		vote->startTime = _levelTime;
		return move(vote);
	}
	return nullptr;
}
