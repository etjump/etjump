#pragma once
#include <string>
#include <memory>
#include "etj_iconfig_string_events_handler.h"

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

		explicit ClientVoteManager(
			std::shared_ptr<IClientCommandsHandler> clientCommandsHandler, 
			std::shared_ptr<IClientCommandsHandler> serverCommandsHandler,
			std::shared_ptr<IConfigStringEventsHandler> configStringEventsHandler
		);

		~ClientVoteManager();
	private:
		std::shared_ptr<IClientCommandsHandler> _clientCommandsHandler;
		std::shared_ptr<IClientCommandsHandler> _serverCommandsHandler;
		std::shared_ptr<IConfigStringEventsHandler> _configStringEventsHandler;
		int _eventHandlerId;
		int numVoted;
		// Handles the vote based on the currently displayed vote
		// Returns the success/error message
		std::string handleVote(VoteParameter voteParameter);
		void configStringModified(int configString, const std::string value);
	};
}



