#pragma once
#include <vector>
#include <array>

namespace ETJump
{
	class DeathrunSystem
	{
	public:
		DeathrunSystem();
		~DeathrunSystem();
		/**
		 * Creates a checkpoint
		 * @returns id for the created checkpoint
		 */
		int createCheckpoint();
		/**
		 * Activates run for the player
		 * @param clientNum ID of the activating player
		 * @returns did a new run start
		 */
		bool hitStart(int clientNum);
		/**
		 * Activates checkpoint for player
		 * @param checkpointId ID of the activated checkpoint
		 * @param clientNum ID of the activating player
		 * @returns was it a new checkpoint
		 */
		bool hitCheckpoint(int checkpointId, int clientNum);
		/**
		 * Stops the run for player
		 * @param clientNum
		 * @returns score
		 */
		int hitEnd(int clientNum);
	private:
		struct RunStatus
		{
			RunStatus()
			{
				checkpointStatuses.clear();
				active = false;
			}
			std::vector<bool> checkpointStatuses;
			bool active;
		};
		/**
		 * Checks if run is active for player
		 * @param clientNum ID of the activating player
		 * @returns is run active for player
		 */
		bool isActive(int clientNum);
		/**
		 * Resets run score and starts a new run
		 * @param clientNum activating player ID
		 */
		void setActive(int clientNum);
		/**
		 * Has client already reached specific checkpoint
		 * @param checkpointId 
		 * @param clientNum 
		 * @returns 
		 */
		bool alreadyReached(int checkpointId, int clientNum);
		/**
		 * Gets score for specific player
		 * @param clientNum
		 * @returns Score
		 */
		int getScore(int clientNum);

		static const int MaxClients = 64;
		std::array<RunStatus, MaxClients> _runStatuses;
	};
}



