#pragma once
#include <boost/container/allocator_traits.hpp>
#include <string>
#include <array>
#include <vector>
#include <queue>
#include <memory>
#include "etj_log.h"
#include "etj_event_aggregator.h"

namespace ETJump
{
	class IMapQueries;
	class ServerCommandsHandler;

	class VoteSystem
	{
	public:
		static const int MAX_VOTERS = 64;

		struct VoteSystemOptions
		{
			VoteSystemOptions(): maxRevotes(5), voteDurationMs(30000)
			{
				
			}
			int maxRevotes;
			int voteDurationMs;
		};

		enum class VoteType
		{
			MapRestart,
			Map,
			RandomMap
		};

		struct Vote
		{
			// level.time when the vote was started
			int startTime;

			// type of the vote
			VoteType type;

			// who voted
			std::string voter;

			std::string toString();
		};

		struct MapVote : public Vote
		{
			std::string map;
		};

		enum class VoteStatus
		{
			No,
			Yes,
			NotYetVoted
		};

		// a player
		struct Voter
		{
			VoteStatus status;
			// how many times client has revoted during the current vote
			int numRevotes;
		};

		struct VoteParseResult
		{
			VoteStatus voted;
			bool success;
		};

		enum class QueuedVote
		{
			No,
			Yes
		};

		explicit VoteSystem(ServerCommandsHandler *commandsHandler, EventAggregator *eventAggregator, IMapQueries *mapQueries, VoteSystemOptions options = VoteSystemOptions());
		~VoteSystem();
	private:
		// initialize the client commands that the vote system will handle
		void initCommands();

		// initialize the server event listening
		void initEventListening();

		// initialize the voter structures
		void initVoters();

		// client is trying to vote 
		void vote(int clientNum, const std::vector<std::string>& args);

		// checks whether client voted for yes or no
		static VoteParseResult parseVoteArgs(const std::vector<std::string>& args);

		// for registering the commands
		ServerCommandsHandler *_commandsHandler;

		// for querying map data
		IMapQueries *_mapQueries;

		// for subscribing to runframe
		EventAggregator* _eventAggregator;

		// event handles returned from event aggregator
		std::vector<int> _eventHandles;

		// client wants to start a new vote
		void callVote(int clientNum, const std::vector<std::string>& args);

		// handles the printing after a vote has been queued
		void displayVoteQueueResult(int clientNum, QueuedVote voteWasQueued);

		// client wants to start a new map vote
		void mapVote(int clientNum, const std::vector<std::string>& args);

		// creates a simple vote (that requires no parameters) and adds it to the queue/current vote
		void createSimpleVote(int clientNum, VoteType type);

		// calls a vote if none exist, else queues it
		QueuedVote callOrQueueVote(const std::unique_ptr<Vote> vote);

		// gets the next vote or nullptr from vote queue
		std::unique_ptr<Vote> popNextVote();

		// checks the current vote status
		void checkVote();

		// list of votes. It is possible to queue multiple votes
		std::queue<std::unique_ptr<Vote>> _voteQueue;

		// current vote
		std::unique_ptr<Vote> _currentVote;

		// current time
		int _levelTime;

		// player's vote state
		std::array<Voter, MAX_VOTERS> _voters;

		// options
		VoteSystemOptions _options;
	};
}

