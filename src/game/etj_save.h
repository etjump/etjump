#pragma once
#include <map>
#include <vector>
#include <boost/circular_buffer.hpp>

#include "etj_local.h"


namespace ETJump
{
	class ISession;

	class SaveSystem {
	public:

		explicit SaveSystem(ISession *session, ServerCommandsHandler *commandsHandler);
		~SaveSystem();

		static const int MAX_SAVED_POSITIONS = 3;
		static const int MAX_BACKUP_POSITIONS = 3;

		struct SavePosition
		{
			bool isValid;
			vec3_t origin;
			vec3_t vangles;
		};

		struct Client
		{
			Client();

			SavePosition alliesSavedPositions[MAX_SAVED_POSITIONS];
			boost::circular_buffer<SavePosition> alliesBackupPositions;

			SavePosition axisSavedPositions[MAX_SAVED_POSITIONS];
			boost::circular_buffer<SavePosition> axisBackupPositions;
		};

		struct DisconnectedClient
		{
			DisconnectedClient();

			// Allies saved positions at the time of disconnect
			SavePosition alliesSavedPositions[MAX_SAVED_POSITIONS];
			// Axis saved positions at the time of disconnect
			SavePosition axisSavedPositions[MAX_SAVED_POSITIONS];

			// So called "map ident"
			int progression;
		};

		// Saves current position
		void save(gentity_t *ent, const std::vector<std::string>& arguments);

		// Loads position
		void load(gentity_t *ent, const std::vector<std::string>& arguments);

		// Saves position, does not check for anything
		void forceSave(gentity_t *location, gentity_t *ent);

		// Loads backup position
		void loadBackupPosition(gentity_t *ent, const std::vector<std::string>& arguments);

		// resets all clients positions
		void reset();

		// Resets targets positions
		void resetSavedPositions(gentity_t *ent);

		// Saves positions to db on disconnect
		void savePositionsToDatabase(gentity_t *ent);

		// Loads positions from db on dc
		void loadPositionsFromDatabase(gentity_t *ent);

	private:
		// Saves backup position
		void saveBackupPosition(gentity_t *ent, SavePosition *pos);

		// All clients' save related data
		Client clients_[MAX_CLIENTS];

		// Disconnected clients saved position data
		std::map<std::string, DisconnectedClient> savedPositions;
		ISession* _session;
		ServerCommandsHandler* _commandsHandler;
	};
}

