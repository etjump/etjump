#pragma once
#include <vector>
#include <array>

namespace ETJump
{
	class DeathrunSystem
	{
	public:
		enum class PrintLocation
		{
			Unspecified,
			Console,
			Chat,
			Center,
			Left
		};

		struct CheckpointData
		{
			PrintLocation location;
			std::string message;
			std::string soundPath;
		};

		DeathrunSystem();
		~DeathrunSystem();
		/**
		 * Adds a start message that will be displayed when the run starts
		 * @param startMessage
		 */
		void addStartMessage(const std::string& startMessage);
		/**
		 * Adds the default message. Displays the message on checkpoint hit if no custom
		 * message was specified
		 * @param defaultMessage
		 */
		void addDefaultCheckpointMessage(const std::string& defaultMessage);
		/**
		 * Adds the default sound. Plays the sound when checkpoint is hit if the checkpoint
		 * has no custom sound
		 * @param defaultSoundPath
		 */
		void addDefaultSoundPath(const std::string& defaultSoundPath);
		/**
		 * Adds the default location for start + checkpoints
		 * @param location
		 */
		void addStartAndCheckpointMessageLocation(PrintLocation location);
		/**
		 * Creates a checkpoint
		 * @returns id for the created checkpoint
		 */
		int createCheckpoint(PrintLocation location, const std::string& message, const std::string& soundPath);
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
		/**
		* Gets score for specific player
		* @param clientNum
		* @returns Score
		*/
		int getScore(int clientNum);
		/**
		* Returns the default print location 
		*/
		PrintLocation getPrintLocation() const;
		/**
		 * Returns the print location for specific checkpoint 
		 * @param checkpointId
		 */
		PrintLocation getPrintLocation(int checkpointId);
		/**
		 * Returns the start message
		 * @returns start message
		 */
		std::string getStartMessage() const;
		/**
		 * Returns the printable message for checkpoint
		 * @param checkpointId
		 * @returns 
		 */
		std::string getCheckpointMessage(int checkpointId) const;
		/**
		 * Returns the sound path for checkpoint
		 * @param checkpointId
		 * @returns 
		 */
		std::string getSoundPath(int checkpointId) const;
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

		PrintLocation _defaultLocation;
		std::string _startMessage;
		std::string _defaultMessage;
		std::string _defaultSoundPath;
		std::vector<CheckpointData> _checkpointData;
		static const int MaxClients = 64;
		std::array<RunStatus, MaxClients> _runStatuses;
	};
}



