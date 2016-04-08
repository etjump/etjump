#pragma once

#include <vector>
#include <array>

namespace ETJump
{
	class ProgressionTrackers
	{
	public:
		static const char *ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET;
		static const int ProgressionTrackerValueNotSet = -1;
		struct ProgressionTrackerKeys
		{
			char *equal;
			char *greaterThan;
			char *lessThan;
			char *set;
			char *setIf;
			char *increment;
			char *incrementIf;
		};
		
		static const int MaxProgressionTrackers = 10;
		struct ProgressionTracker
		{
			ProgressionTracker()
			{
				for (auto & v : equal) v = -1;
				for (auto & v : greaterThan) v = -1;
				for (auto & v : lessThan) v = -1;
				for (auto & v : set) v = -1;
				for (auto & v : setIf) v = -1;
				for (auto & v : increment) v = 0;
				for (auto & v : incrementIf) v = 0;
			}
			int equal[MaxProgressionTrackers];
			int greaterThan[MaxProgressionTrackers];
			int lessThan[MaxProgressionTrackers];
			int set[MaxProgressionTrackers];
			int setIf[MaxProgressionTrackers];
			int increment[MaxProgressionTrackers];
			int incrementIf[MaxProgressionTrackers];
		};

		ProgressionTrackers();
		~ProgressionTrackers();
		void printParserErrors(const std::vector<std::string>& errors, const std::string& text);
		std::vector<ProgressionTrackerParser::IndexValuePair> parseKey(const std::string& key);
		void updateTracker(std::vector<ProgressionTrackerParser::IndexValuePair> pairs, int tracker[MaxProgressionTrackers]);
		int registerTracker(ProgressionTrackerKeys keys);
		void useTargetTracker(gentity_t* ent, gentity_t* other, gentity_t* activator);
		void useTriggerTracker(gentity_t* ent, gentity_t* other, gentity_t* activator);
	private:
		std::vector<ProgressionTracker> _progressionTrackers;
	};
}



