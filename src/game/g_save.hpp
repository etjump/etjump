#ifndef G_SAVE_H
#define G_SAVE_H

#include <map>
#include <string>
#include <boost/circular_buffer.hpp>

#include "g_local.hpp"

class SaveSystem {
public:

    static const unsigned MAX_SAVED_POSITIONS = 3;
    static const unsigned MAX_BACKUP_POSITIONS = 3;

    struct SavePosition {
	    bool	    isValid;
	    vec3_t		origin;
	    vec3_t		vangles;
    };

    struct Client {
        Client();

        SavePosition alliesSavedPositions[MAX_SAVED_POSITIONS];
        boost::circular_buffer<SavePosition> alliesBackupPositions;

        SavePosition axisSavedPositions[MAX_SAVED_POSITIONS];
        boost::circular_buffer<SavePosition> axisBackupPositions;
    };

    struct DisconnectedClient {
        DisconnectedClient();

        // Allies saved positions at the time of disconnect
        SavePosition alliesSavedPositions[MAX_SAVED_POSITIONS];
        // Axis saved positions at the time of disconnect
        SavePosition axisSavedPositions[MAX_SAVED_POSITIONS];

        // So called "map ident"
        int progression;
    };

    // Saves current position
    static void Save(gentity_t *ent);

    // Loads position
    static void Load(gentity_t *ent);

    // Saves position, does not check for anything
    static void ForceSave(gentity_t *location, gentity_t *ent);
    
    // Loads backup position
    static void LoadBackupPosition(gentity_t *ent);

    // resets all clients positions
    static void Reset();

    // Resets targets positions
    static void ResetSavedPositions(gentity_t *ent);

    // Saves positions to db on disconnect
    static void SavePositionsToDatabase(gentity_t *ent);

    // Loads positions from db on dc
    static void LoadPositionsFromDatabase(gentity_t *ent);

    // Prints entire db
    static void Print( gentity_t *ent );
private:
    // Saves backup position
    static void SaveBackupPosition(gentity_t *ent, SavePosition *pos);

    // All clients' save related data
    static Client clients_[MAX_CLIENTS];

    // Disconnected clients saved position data
    static std::map<std::string, DisconnectedClient> savedPositions;
};

#endif
