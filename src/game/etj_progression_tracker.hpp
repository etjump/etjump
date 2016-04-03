#pragma once

#include <vector>

namespace ETJump
{
	class ProgressionTrackers
	{
	public:
		static const char *ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET;
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
		
		static const int MAX_PROGRESSION_TRACKERS = 10;
		struct ProgressionTracker
		{
			ProgressionTracker()
			{
				for (auto & v : equal) v = -1;
				for (auto & v : greaterThan) v = -1;
				for (auto & v : lessThan) v = -1;
				for (auto & v : set) v = -1;
				for (auto & v : setIf) v = -1;
				for (auto & v : increment) v = -1;
				for (auto & v : incrementIf) v = -1;
			}
			int equal[MAX_PROGRESSION_TRACKERS];
			int greaterThan[MAX_PROGRESSION_TRACKERS];
			int lessThan[MAX_PROGRESSION_TRACKERS];
			int set[MAX_PROGRESSION_TRACKERS];
			int setIf[MAX_PROGRESSION_TRACKERS];
			int increment[MAX_PROGRESSION_TRACKERS];
			int incrementIf[MAX_PROGRESSION_TRACKERS];
		};

		ProgressionTrackers();
		~ProgressionTrackers();
		void printParserErrors(const std::vector<std::string>& errors, const std::string& text);
		std::vector<ProgressionTrackerParser::IndexValuePair> parseKey(const std::string& key);
		void updateTracker(std::vector<ProgressionTrackerParser::IndexValuePair> pairs, int tracker[MAX_PROGRESSION_TRACKERS]);
		int registerTracker(ProgressionTrackerKeys keys);
		void useEntity(gentity_t* ent, gentity_t* other, gentity_t* activator);

	private:
		std::vector<ProgressionTracker> _progressionTrackers;
	};
}



