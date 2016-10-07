#include "etj_vote_system.h"
#include "etj_imap_facade.h"
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

ETJump::VoteSystem::MapVote::MapVote(IMapFacade* mapFacade): _mapFacade(mapFacade)
{
}

void ETJump::VoteSystem::MapVote::execute()
{
	_mapFacade->changeMap(map);
}

void ETJump::VoteSystem::MapRestartVote::execute()
{
}

void ETJump::VoteSystem::RandomMapVote::execute()
{
}

ETJump::VoteSystem::VoteSystem(ServerCommandsHandler* commandsHandler,
	EventAggregator* eventAggregator,
	IMapFacade* mapFacade,
	IPlayerQueries* playerQueries,
	ICvarManager* cvars,
	IVoteStrategy* voteStrategy,
	VoteSystemOptions options):

	_commandsHandler(commandsHandler),
	_mapFacade(mapFacade),
	_playerQueries(playerQueries),
	_cvarManager(cvars),
	_eventAggregator(eventAggregator),
	_eventHandles{},
	_voteStrategy(voteStrategy),
	_voteQueue{},
	_currentVote(nullptr),
	_levelTime(0)
{
	if (_commandsHandler == nullptr)
	{
		Log::error("commandsHandler is null.");
		return;
	}

	if (_mapFacade == nullptr)
	{
		Log::error("mapFacade is null.");
		return;
	}

	if (_eventAggregator == nullptr)
	{
		Log::error("eventAggregator is null.");
		return;
	}

	if (_playerQueries == nullptr)
	{
		Log::error("playerQueries is null.");
		return;
	}

	if (_cvarManager == nullptr)
	{
		Log::error("cvars is null.");
		return;
	}

	if (_voteStrategy == nullptr)
	{
		Log::error("voteStrategy is null.");
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
	for (const auto& h : _eventHandles)
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
	_eventHandles.push_back(_eventAggregator->subscribe(EventAggregator::ServerEventType::RunFrame,
		[&](const EventAggregator::Payload* payload)
		{
			const EventAggregator::RunFramePayload* pl = reinterpret_cast<const EventAggregator::RunFramePayload*>(payload);
			_levelTime = pl->levelTime;

			if (!_currentVote)
			{
				return;
			}

			auto numNo = 0;
			auto numYes = 0;
			for (const auto& voter : _voters)
			{
				if (voter.status == VoteStatus::No) ++numNo;
				if (voter.status == VoteStatus::Yes) ++numYes;
			}

			auto voteResult = _voteStrategy->getVoteResult(
				IVoteStrategy::Input(
					_cvarManager->getInteger(Cvars::Vote::Percent),
					numYes,
					numNo,
					_playerQueries->playingPlayerCount(),
					_playerQueries->connectedPlayerCount(),
					_levelTime - _currentVote->startTime,
					_options.voteDurationMs,
					3000 // wait for a minimum of 3s before passing/failing vote
				));

			if (voteResult.status == IVoteStrategy::Status::Wait)
			{
				return;
			}

			if (voteResult.status == IVoteStrategy::Status::Fail)
			{
				Printer::BroadCastBannerMessage(boost::format("^1Vote failed: ^7(%s)") % _currentVote->toString());
			}
			else if (voteResult.status == IVoteStrategy::Status::Pass)
			{
				Printer::BroadCastBannerMessage(boost::format("^2Vote passed. ^7(%s)") % _currentVote->toString());

				// special case for map change and restart, we need to clear all votes to prevent
				// any from occurring after map changes and server kicks everyone out
				// currently this is the only case but we might have some other vote types in the future
				if (_currentVote->type == VoteType::Map || _currentVote->type == VoteType::MapRestart || _currentVote->type == VoteType::RandomMap)
				{
					_voteQueue = std::queue<std::unique_ptr<Vote>>();
				}

				_currentVote->execute();
			}

			_currentVote = popNextVote();
			if (_currentVote != nullptr)
			{
				Printer::BroadcastCenterMessage(boost::format("%s called a vote: %s.") % _playerQueries->name(_currentVote->voterClientNum) % _currentVote->toString());
				_voters[_currentVote->voterClientNum].status = VoteStatus::Yes;
			}
		}));
}

void ETJump::VoteSystem::initVoters()
{
	for (auto& v : _voters)
	{
		v.numRevotes = 0;
		v.status = VoteStatus::NotYetVoted;
	}
}

void ETJump::VoteSystem::vote(int clientNum, const std::vector<std::string>& args)
{
	const auto usage = "^3usage: ^7/vote <yes|no>\n";

	auto team = _playerQueries->team(clientNum);
	if (team == Team::Free || team == Team::Spectator)
	{
		Printer::SendConsoleMessage(clientNum, "^3Cannot vote as a spectator.\n");
		return;
	}

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
	auto team = _playerQueries->team(clientNum);
	if (team == Team::Free || team == Team::Spectator)
	{
		Printer::SendConsoleMessage(clientNum, "^3Cannot vote as a spectator.\n");
		return;
	}

	if (args.size() == 1)
	{
		Printer::SendConsoleMessage(clientNum, "^3usage: ^7/callvote <vote> <additional parameters>\n");
		return;
	}

	auto type = boost::to_lower_copy(args[1]);
	CreateVoteResult vote;
	if (type == "map")
	{
		vote = createMapVote(clientNum, args);
	}
	else if (type == "randommap")
	{
		vote = createSimpleVote(clientNum, VoteType::RandomMap);
	}
	else if (type == "maprestart")
	{
		vote = createSimpleVote(clientNum, VoteType::MapRestart);
	}
	else
	{
		Printer::SendConsoleMessage(clientNum, (boost::format("^3error: ^7unknown vote type: %s.\n") % args[1]).str());
		return;
	}
	vote.vote->voterClientNum = clientNum;

	if (!vote.vote)
	{
		Printer::SendConsoleMessage(clientNum, vote.error);
	}
	else
	{
		displayVoteQueueResult(clientNum, callOrQueueVote(move(vote.vote)));
		_voters[clientNum].status = VoteStatus::Yes;
	}
}

void ETJump::VoteSystem::displayVoteQueueResult(int clientNum, QueuedVote voteWasQueued) const
{
	if (voteWasQueued == QueuedVote::Yes)
	{
		Printer::SendConsoleMessage(clientNum, boost::format("^gvote:^7 vote is currently in progress. Your vote has been added to the vote queue. There are currently %d votes in the queue.\n") % _voteQueue.size());
	}
	else
	{
		Printer::BroadcastCenterMessage(boost::format("^7%s ^7called a vote: %s.") % _playerQueries->name(clientNum) % _currentVote.get()->toString());
	}
}

ETJump::VoteSystem::CreateVoteResult ETJump::VoteSystem::createMapVote(int clientNum, const std::vector<std::string>& args) const
{
	std::unique_ptr<Vote> vote(new MapVote(_mapFacade));
	vote->type = VoteType::Map;

	if (args.size() != 3)
	{
		return {nullptr, "^3usage: /callvote map <map name>\n"};
	}

	auto map = boost::to_lower_copy(args[2]);
	auto matches = _mapFacade->matches(map);
	if (matches.size() == 0)
	{
		return {nullptr, (boost::format("^1error: ^7could not find a matching map with name: \"%s\"\n") % map).str()};
	}

	if (matches.size() > 1)
	{
		return {nullptr, (boost::format("^1error: ^7found multiple matching maps:\n* %s\n") % boost::join(matches, "\n* ")).str()};
	}

	reinterpret_cast<MapVote*>(vote.get())->map = matches[0];

	return {move(vote), ""};
}

ETJump::VoteSystem::CreateVoteResult ETJump::VoteSystem::createSimpleVote(int clientNum, VoteType type) const
{
	std::unique_ptr<Vote> vote;
	switch (type)
	{
	case VoteType::MapRestart:
		vote = std::unique_ptr<Vote>(new MapRestartVote());
		break;
	case VoteType::RandomMap:
		vote = std::unique_ptr<Vote>(new RandomMapVote());
		break;
	default:
		Log::error(boost::format("Unknown simple vote type: %d") % static_cast<int>(type));
	}
	vote->type = type;
	return {move(vote), ""};
}

ETJump::VoteSystem::QueuedVote ETJump::VoteSystem::callOrQueueVote(std::unique_ptr<Vote> vote)
{
	if (_currentVote == nullptr)
	{
		resetPlayersVotes();

		vote->startTime = _levelTime;
		_currentVote = move(vote);
		return QueuedVote::No;
	}

	_voteQueue.push(move(vote));
	return QueuedVote::Yes;
}

void ETJump::VoteSystem::resetPlayersVotes()
{
	for (auto& v : _voters)
	{
		v.status = VoteStatus::NotYetVoted;
		v.numRevotes = 0;
	}
}

std::unique_ptr<ETJump::VoteSystem::Vote> ETJump::VoteSystem::popNextVote()
{
	if (_voteQueue.size() > 0)
	{
		resetPlayersVotes();

		auto vote = move(_voteQueue.front());
		_voteQueue.pop();
		vote->startTime = _levelTime;
		return move(vote);
	}
	return nullptr;
}

ETJump::VoteSystem::VoteRequirements ETJump::VoteSystem::getVoteRequirements() const
{
	auto requiredVotePercent = _cvarManager->getInteger(Cvars::Vote::Percent);
	auto result = VoteRequirements{};
	result.votePercent = requiredVotePercent;
	return result;
}
