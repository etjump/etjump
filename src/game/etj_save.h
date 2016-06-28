#ifndef G_SAVE_H
#define G_SAVE_H

#include <map>
#include <string>
#include <boost/circular_buffer.hpp>

#include "etj_local.h"

class Session;

class SaveSystem {
public:

	SaveSystem();
	/*SaveSystem( IGuid *guidInterface );*/
	~SaveSystem();

	static const int MAX_SAVED_POSITIONS  = 3;
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
	void Save(gentity_t *ent);

	// Loads position
	void Load(gentity_t *ent);

	// Saves position, does not check for anything
	void ForceSave(gentity_t *location, gentity_t *ent);

	// Loads backup position
	void LoadBackupPosition(gentity_t *ent);

	// resets all clients positions
	void Reset();

	// Resets targets positions
	void ResetSavedPositions(gentity_t *ent);

	// Saves positions to db on disconnect
	void SavePositionsToDatabase(gentity_t *ent);

	// Loads positions from db on dc
	void LoadPositionsFromDatabase(gentity_t *ent);

	// Prints entire db
	void Print(gentity_t *ent);
private:
	// Saves backup position
	void SaveBackupPosition(gentity_t *ent, SavePosition *pos);

	// All clients' save related data
	Client clients_[MAX_CLIENTS];

	// Disconnected clients saved position data
	std::map<std::string, DisconnectedClient> savedPositions;
};

#endif
