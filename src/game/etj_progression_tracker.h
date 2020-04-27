/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <haapanen.jussi@gmail.com>
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
			char *notEqual;
			char *greaterThan;
			char *lessThan;
			char *set;
			char *setIf;
			char *increment;
			char *incrementIf;
		};
		
		static const int MaxProgressionTrackers = 50;
		struct ProgressionTracker
		{
			ProgressionTracker()
			{
				for (auto & v : equal) v = -1;
				for (auto & v : notEqual) v = -1;
				for (auto & v : greaterThan) v = -1;
				for (auto & v : lessThan) v = -1;
				for (auto & v : set) v = -1;
				for (auto & v : setIf) v = -1;
				for (auto & v : increment) v = 0;
				for (auto & v : incrementIf) v = 0;
			}
			int equal[MaxProgressionTrackers];
			int notEqual[MaxProgressionTrackers];
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
		void useTriggerTracker(gentity_t* ent, gentity_t* activator);
	private:
		void useTracker(gentity_t *ent, gentity_t *activator, const ProgressionTracker& tracker);

		std::vector<ProgressionTracker> _progressionTrackers;
	};
}



