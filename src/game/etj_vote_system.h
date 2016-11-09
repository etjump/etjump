#pragma once
#include <boost/container/allocator_traits.hpp>
#include <string>
#include <array>
#include <vector>
#include <queue>
#include <memory>
#include "etj_log.h"
#include "etj_event_aggregator.h"
#include "etj_iplayer_queries.h"
#include "etj_icvar_manager.h"
#include "etj_ivote_strategy.h"
#include "etj_imap_facade.h"

namespace ETJump
{
	class ServerCommandsHandler;

	namespace Cvars
	{
		namespace Vote
		{
			const auto Percent = "vote_percent";
			const auto Limit = "vote_limit";
		}
	}

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

		class Vote
		{
		public:
			virtual ~Vote()
			{
			}

			// level.time when the vote was started
			int startTime;

			// type of the vote
			VoteType type;

			// who voted
			int voterClientNum;

			std::string toString();
			
			// what to do with the vote
			virtual void execute() = 0;
		};

		class MapVote : public Vote
		{
		public:
			explicit MapVote(std::shared_ptr<IMapFacade> mapFacade);
			std::string map;

			void execute() override;
		private:
			std::shared_ptr<IMapFacade> _mapFacade;
		};

		class MapRestartVote : public Vote
		{
		public:
			void execute() override;
		};

		class RandomMapVote : public Vote
		{
		public:
			void execute() override;
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

		/**
		 * Current vote requirements based on cvars / state of the game
		 */
		struct VoteRequirements
		{
			int votePercent;
		};

		/**
		 * Result from a vote creation function
		 */
		struct CreateVoteResult
		{
			std::unique_ptr<Vote> vote;
			std::string error;
		};

		explicit VoteSystem(std::shared_ptr<ServerCommandsHandler> commandsHandler, 
			std::shared_ptr<EventAggregator> eventAggregator, 
			std::shared_ptr<IMapFacade> mapFacade,
			std::shared_ptr<IPlayerQueries> playerQueries,
			std::shared_ptr<ICvarManager> cvars,
			std::shared_ptr<IVoteStrategy> voteStrategy,
				VoteSystemOptions options = VoteSystemOptions());
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
		std::shared_ptr<ServerCommandsHandler> _commandsHandler;

		// for querying map data
		std::shared_ptr<IMapFacade> _mapFacade;

		// for querying player data
		std::shared_ptr<IPlayerQueries> _playerQueries;

		// for querying cvar values
		std::shared_ptr<ICvarManager> _cvarManager;

		// for subscribing to runframe
		std::shared_ptr<EventAggregator> _eventAggregator;

		// event handles returned from event aggregator
		std::vector<int> _eventHandles;

		// logic on how to pass votes
		std::shared_ptr<IVoteStrategy> _voteStrategy;

		// client wants to start a new vote
		void callVote(int clientNum, const std::vector<std::string>& args);

		// handles the printing after a vote has been queued
		void displayVoteQueueResult(int clientNum, QueuedVote voteWasQueued) const;

		// client wants to start a new map vote
		CreateVoteResult createMapVote(int clientNum, const std::vector<std::string>& args) const;

		// creates a simple vote (that requires no parameters) and adds it to the queue/current vote
		CreateVoteResult createSimpleVote(int clientNum, VoteType type) const;

		// calls a vote if none exist, else queues it
		QueuedVote callOrQueueVote(const std::unique_ptr<Vote> vote);

		// reset the vote status of each player
		void resetPlayersVotes();

		// gets the next vote or nullptr from vote queue
		std::unique_ptr<Vote> popNextVote();

		// get vote requirements that are currently active
		VoteRequirements getVoteRequirements() const;

		// current vote's requirements
		VoteRequirements _voteRequirements;

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

