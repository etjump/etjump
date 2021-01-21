/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <zero@etjump.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef G_SAVE_H
#define G_SAVE_H

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <map>
#include <string>
#include <boost/circular_buffer.hpp>

#include "etj_local.h"

class Session;

namespace ETJump
{
	class SaveSystem {
	public:

		SaveSystem(const std::shared_ptr<Session> session);
		/*SaveSystem( IGuid *guidInterface );*/
		~SaveSystem();

		static const int MAX_SAVED_POSITIONS = 3;
		static const int MAX_BACKUP_POSITIONS = 3;

		enum SaveStance
		{
			Stand,
			Crouch,
			Prone
		};

		struct SavePosition
		{
			SavePosition() : isValid(false), origin{ 0,0,0 }, vangles{ 0,0,0 }, stance(SaveStance::Stand) {}
			bool isValid;
			vec3_t origin;
			vec3_t vangles;
			SaveStance stance;
		};

		struct Client
		{
			Client();

			SavePosition alliesSavedPositions[MAX_SAVED_POSITIONS];
			boost::circular_buffer<SavePosition> alliesBackupPositions;
			SavePosition alliesLastLoadPosition;

			SavePosition axisSavedPositions[MAX_SAVED_POSITIONS];
			boost::circular_buffer<SavePosition> axisBackupPositions;
			SavePosition axisLastLoadPosition;

			// contains a couple of extra positions for TEAM_SPEC and TEAM_FREE,
			// but simplifies the accessing code
			SavePosition quickDeployPositions[TEAM_NUM_TEAMS];
		};

		struct DisconnectedClient
		{
			DisconnectedClient();

			// Allies saved positions at the time of disconnect
			SavePosition alliesSavedPositions[MAX_SAVED_POSITIONS];
			// Axis saved positions at the time of disconnect
			SavePosition axisSavedPositions[MAX_SAVED_POSITIONS];

			// Last load positions
			SavePosition axisLastLoadPosition;
			SavePosition alliesLastLoadPosition;


			// So called "map ident"
			int progression;
		};

		enum class SaveLoadRestrictions
		{
			Default = 0,
			Move = 1 << 0,
			Dead = 1 << 1,
		};

		// Saves current position
		void save(gentity_t *ent);

		// Loads position
		void load(gentity_t *ent);

		// Saves position, does not check for anything
		void forceSave(gentity_t *location, gentity_t *ent);

		// Loads backup position
		void loadBackupPosition(gentity_t *ent);

		// UnLoad - revert last "load" command
		void unload(gentity_t* ent);

		// Saves last position client loaded from
		void saveLastLoadPos(gentity_t* ent);

		// resets all clients positions
		void reset();

		// Resets targets positions
		void resetSavedPositions(gentity_t *ent);

		// Saves positions to db on disconnect
		void savePositionsToDatabase(gentity_t *ent);

		// Loads positions from db on dc
		void loadPositionsFromDatabase(gentity_t *ent);

		void storeTeamQuickDeployPosition(gentity_t *ent, team_t team);
		void loadTeamQuickDeployPosition(gentity_t *ent, team_t team);
		void loadOnceTeamQuickDeployPosition(gentity_t *ent, team_t team);

	private:
		// Saves backup position
		void saveBackupPosition(gentity_t *ent, SavePosition *pos);

		// copies player positional info to target position
		void storePosition(gclient_s* client, SavePosition *pos);

		// Teleports player to the saved position
		static void teleportPlayer(gentity_t* ent, SavePosition* pos);

		SavePosition* getValidTeamQuickDeploySave(gentity_t *ent, team_t team);

		void restoreStanceFromSave(gentity_t *ent, SavePosition *pos);

		SavePosition* getValidTeamSaveForSlot(gentity_t *ent, team_t team, int slot);
		
		SavePosition* getValidTeamUnloadPos(gentity_t* ent, team_t team);

		// All clients' save related data
		Client _clients[MAX_CLIENTS];

		// Disconnected clients saved position data
		std::map<std::string, DisconnectedClient> _savedPositions;

		// Interface to get player guid
		const std::shared_ptr<Session> _session;
		/*IGuid *guidInterface_;*/
	};
}


#endif
