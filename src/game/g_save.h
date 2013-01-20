#ifndef G_SAVE_H
#define G_SAVE_H

#include <boost/circular_buffer.hpp>

extern "C" {
#include "g_local.h"
}

class Save {
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

    // Saves current position
    void save(gentity_t *ent);

    // Loads position
    void load(gentity_t *ent);

    // Saves position, does not check for anything
    void forceSave(gentity_t *location, gentity_t *ent);
    
    // Loads backup position
    void loadBackupPosition(gentity_t *ent);

    // resets all clients positions
    void reset();

    // Resets targets positions
    void resetSavedPositions(gentity_t *ent);
private:
    // Saves backup position
    void saveBackupPosition(gentity_t *ent, SavePosition *pos);

    // All clients' save related data
    Client clients_[MAX_CLIENTS];
};

#endif