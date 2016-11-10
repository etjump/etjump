#pragma once
#include <string>
#include <memory>

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

		enum class QuestionType
		{
			Vote,
			FireteamInvitation
		};

		// A vote or a fireteam invitation
		struct Question
		{
			QuestionType type;
			std::string message;
		};

		explicit ClientVoteManager(std::shared_ptr<IClientCommandsHandler> clientCommandsHandler);

		~ClientVoteManager();
	private:
		std::shared_ptr<IClientCommandsHandler> _clientCommandsHandler;
		int numVoted;

		// Handles the vote based on the currently displayed vote
		// Returns the success/error message
		std::string handleVote(VoteParameter voteParameter);

	};
}



