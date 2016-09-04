#pragma once
#include <boost/container/allocator_traits.hpp>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include "etj_log.h"

namespace ETJump
{
	class IMapQueries;
	class ServerCommandsHandler;

	class VoteSystem
	{
	public:
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

			std::string toString();
		};

		struct MapVote : public Vote
		{
			std::string map;
		};

		enum class Voted
		{
			No,
			Yes
		};

		struct VoteParseResult
		{
			Voted voted;
			bool success;
		};

		enum class QueuedVote
		{
			No,
			Yes
		};

		explicit VoteSystem(ServerCommandsHandler *commandsHandler, IMapQueries *mapQueries);
		~VoteSystem();

		// update the state on every frame
		void runFrame(int levelTime);
	private:
		// initialize the client commands that the vote system will handle
		void initCommands();

		// client is trying to vote 
		void vote(int clientNum, const std::vector<std::string>& args);

		// checks whether client voted for yes or no
		static VoteParseResult parseVoteArgs(const std::vector<std::string>& args);

		// for registering the commands
		ServerCommandsHandler *_commandsHandler;

		// for querying map data
		IMapQueries *_mapQueries;

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

		// list of votes. It is possible to queue multiple votes
		std::queue<std::unique_ptr<Vote>> _voteQueue;

		// current vote
		std::unique_ptr<Vote> _currentVote;

		int _levelTime;
	};
}

