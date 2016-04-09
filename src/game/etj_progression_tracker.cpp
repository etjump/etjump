#include <memory>
#include <boost/algorithm/string.hpp>
#include "etj_progression_tracker_parser.hpp"

extern "C" {
#include "g_local.h"
}

#include "etj_progression_tracker.hpp"

const char *ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET = "-1";

ETJump::ProgressionTrackers::ProgressionTrackers()
{
	_progressionTrackers.clear();
}

ETJump::ProgressionTrackers::~ProgressionTrackers()
{
}

void ETJump::ProgressionTrackers::printParserErrors(const std::vector<std::string>& errors, const std::string& text)
{
	auto buffer = "Tracker parse error on line: " + text + "\n";
	buffer += boost::algorithm::join(errors, "\n");
	G_Error(buffer.c_str());
}

void ETJump::ProgressionTrackers::updateTracker(std::vector<ProgressionTrackerParser::IndexValuePair> pairs, int tracker[10])
{
	for (const auto & pair : pairs)
	{
		if (pair.index >= MaxProgressionTrackers)
		{
			G_Error("Tracker error: specified index (%d) is greater than maximum number of trackers (%d)", pair.index, MaxProgressionTrackers);
		}

		tracker[pair.index] = pair.value;
	}
}

std::vector<ETJump::ProgressionTrackerParser::IndexValuePair> ETJump::ProgressionTrackers::parseKey(const std::string& key)
{
	auto parser = ProgressionTrackerParser(key);
	auto errors = parser.getErrors();
	if (errors.size())
	{
		printParserErrors(errors, key);
	}
	return parser.getParsedPairs();
}

int ETJump::ProgressionTrackers::registerTracker(ProgressionTrackerKeys keys)
{
	auto progressionTracker = ProgressionTracker();

	updateTracker(parseKey(keys.equal), progressionTracker.equal);
	updateTracker(parseKey(keys.greaterThan), progressionTracker.greaterThan);
	updateTracker(parseKey(keys.lessThan), progressionTracker.lessThan);
	updateTracker(parseKey(keys.set), progressionTracker.set);
	updateTracker(parseKey(keys.setIf), progressionTracker.setIf);
	updateTracker(parseKey(keys.increment), progressionTracker.increment);
	updateTracker(parseKey(keys.incrementIf), progressionTracker.incrementIf);

	_progressionTrackers.push_back(progressionTracker);
	return _progressionTrackers.size() - 1;
}

namespace ETJump
{
	static std::unique_ptr<ProgressionTrackers> progressionTrackers;
}


void ETJump::ProgressionTrackers::useTracker(gentity_t *ent, gentity_t* activator, const ProgressionTracker& tracker)
{
	auto idx = 0;
	for (auto & v : tracker.set)
	{
		if (v >= 0)
		{
			activator->client->sess.progression[idx] = v;
		}

		++idx;
	}

	idx = 0;
	for (auto & v : tracker.increment)
	{
		if (v != 0)
		{
			activator->client->sess.progression[idx] += v;
		}
		++idx;
	}

	for (idx = 0; idx < MaxProgressionTrackers; ++idx)
	{
		auto clientTracker = activator->client->sess.progression[idx];

		if ((tracker.equal[idx] != ProgressionTrackerValueNotSet && tracker.equal[idx] == clientTracker) ||
			(tracker.lessThan[idx] != ProgressionTrackerValueNotSet && tracker.lessThan[idx] > clientTracker) ||
			(tracker.greaterThan[idx] != ProgressionTrackerValueNotSet && tracker.greaterThan[idx] < clientTracker))
		{
			// activate
			G_UseTargetedEntities(ent, activator);

			if (tracker.setIf[idx] >= 0)
			{
				activator->client->sess.progression[idx] = tracker.setIf[idx];
			}

			if (tracker.incrementIf[idx] != 0)
			{
				activator->client->sess.progression[idx] += tracker.incrementIf[idx];
			}
		}
	}
}

void ETJump::ProgressionTrackers::useTriggerTracker(gentity_t* ent, gentity_t* activator)
{
	auto tracker = _progressionTrackers[ent->key];

	if (!activator || !activator->client)
	{
		return;
	}

	useTracker(ent, activator, tracker);
}

void ETJump::ProgressionTrackers::useTargetTracker(gentity_t* ent, gentity_t* other, gentity_t* activator)
{
	auto tracker = _progressionTrackers[ent->key];

	if (!activator || !activator->client)
	{
		return;
	}

	useTracker(ent, activator, tracker);
}

extern "C" void SP_target_tracker(gentity_t *self)
{
	ETJump::ProgressionTrackers::ProgressionTrackerKeys keys;

	G_SpawnString("tracker_eq", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.equal);
	G_SpawnString("tracker_gt", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.greaterThan);
	G_SpawnString("tracker_lt", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.lessThan);
	G_SpawnString("tracker_set", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.set);
	G_SpawnString("tracker_setIf", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.setIf);
	G_SpawnString("tracker_inc", "0", &keys.increment);
	G_SpawnString("tracker_inc_if", "0", &keys.incrementIf);

	if (!ETJump::progressionTrackers)
	{
		ETJump::progressionTrackers = std::unique_ptr<ETJump::ProgressionTrackers>(new ETJump::ProgressionTrackers);
	}
	self->key = ETJump::progressionTrackers->registerTracker(keys);
	self->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator)
	{
		ETJump::progressionTrackers->useTargetTracker(ent, other, activator);
	};
}

extern "C" void SP_trigger_tracker(gentity_t *self)
{
	InitTrigger(self);
	// just make it same type as trigger multiple for now
	self->s.eType = ET_TRIGGER_MULTIPLE;

	ETJump::ProgressionTrackers::ProgressionTrackerKeys keys;

	G_SpawnString("tracker_eq", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.equal);
	G_SpawnString("tracker_gt", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.greaterThan);
	G_SpawnString("tracker_lt", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.lessThan);
	G_SpawnString("tracker_set", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.set);
	G_SpawnString("tracker_setIf", ETJump::ProgressionTrackers::ETJUMP_PROGRESSION_TRACKER_VALUE_NOT_SET, &keys.setIf);
	G_SpawnString("tracker_inc", "0", &keys.increment);
	G_SpawnString("tracker_inc_if", "0", &keys.incrementIf);

	if (!ETJump::progressionTrackers)
	{
		ETJump::progressionTrackers = std::unique_ptr<ETJump::ProgressionTrackers>(new ETJump::ProgressionTrackers);
	}

	self->key = ETJump::progressionTrackers->registerTracker(keys);
	self->use = [](gentity_t *ent, gentity_t *other, gentity_t *activator)
	{
		ETJump::progressionTrackers->useTriggerTracker(ent, activator);
	};
	self->touch = [](gentity_t *ent, gentity_t *activator, trace_t *trace)
	{
		ETJump::progressionTrackers->useTriggerTracker(ent, activator);
	};
}