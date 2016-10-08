#pragma once
#include <string>

namespace ETJump
{
	class IClientCommandsHandler;

	class ClientVoteManager
	{
	public:
		enum class VoteParameter
		{
			No,
			Yes
		};

		explicit ClientVoteManager(IClientCommandsHandler* clientCommandsHandler);

		~ClientVoteManager();
	private:
		IClientCommandsHandler *_clientCommandsHandler;
		int numVoted;

		// Handles the vote based on the currently displayed vote
		// Returns the success/error message
		std::string handleVote(VoteParameter voteParameter);

	};
}



