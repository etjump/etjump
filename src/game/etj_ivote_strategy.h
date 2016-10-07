#pragma once
namespace ETJump
{
	class IVoteStrategy
	{
	public:
		virtual ~IVoteStrategy()
		{
		}

		// did the vote pass, fail or still pending
		enum class Status
		{
			Pass,
			Fail,
			Wait
		};

		// input parameters for checking if vote should pass or fail
		struct Input
		{
			Input(int votePercent, int yesVotes, int noVotes, int numPlayers, int numConnectedClients, int duration, int maxDuration, int minWait) :
				votePercent(votePercent), yesVotes(yesVotes), noVotes(noVotes), numPlayers(numPlayers), numConnectedClients(numConnectedClients), duration(duration), maxDuration(maxDuration), minWait(minWait) {}
			int votePercent;
			int yesVotes;
			int noVotes;
			int numPlayers;
			int numConnectedClients;
			int duration;
			int maxDuration;
			int minWait;
		};

		struct Result
		{
			Status status;
		};

		// get the result based on the input params
		virtual Result getVoteResult(Input input) = 0;
	};
}
