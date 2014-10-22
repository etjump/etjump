#include "loadrace.hpp"
#include "g_local.hpp"
#include "g_utilities.hpp"

LoadRace::LoadRace(Races *races,  Races::Race race, gentity_t *ent) : races_(races), race_(race), ent_(ent)
{
}

LoadRace::~LoadRace()
{
}

void LoadRace::Execute()
{
    ChatPrintTo(ent_, "^3race: ^7successfully loaded race: " + race_.name);

    races_->CreateLoadedRaceEntities(race_);
}