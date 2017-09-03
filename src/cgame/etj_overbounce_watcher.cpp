#include "etj_client_commands_handler.h"
#include "etj_overbounce_watcher.h"
#include "cg_local.h"

ETJump::OverbounceWatcher::OverbounceWatcher(ClientCommandsHandler* clientCommandsHandler) :
	_clientCommandsHandler{clientCommandsHandler},
	_positions {},
	_current{nullptr}
{
	if (!clientCommandsHandler)
	{
		CG_Error("OverbounceWatcher: clientCommandsHandler is null.\n");
		return;
	}

	_positions.clear();

	clientCommandsHandler->subscribe("ob_save", [&](const std::vector<std::string>& args)
	                                 {
		                                 Coordinate c;
		                                 const auto ps = getPlayerState();
		                                 for (auto i = 0; i < 3; ++i)
		                                 {
			                                 c[i] = ps->origin[i];
		                                 }
		                                 auto name = args.size() > 1 ? args[1] : "default";
		                                 save(name, c);
		                                 CG_Printf("Saved coordinate as \"%s\": (%f, %f, %f)\n", name.c_str(), c[0], c[1], c[2]);
	                                 });

	clientCommandsHandler->subscribe("ob_load", [&](const std::vector<std::string>& args)
	                                 {
		                                 auto name = args.size() > 1 ? args[1] : "default";
		                                 if (!load(name))
		                                 {
			                                 CG_Printf("^3Overbounce watcher: ^7%s has not been saved yet.\n", name.c_str());
			                                 return;
		                                 }

		                                 CG_Printf("Loaded coordinate \"%s\" (%f, %f, %f)\n", name.c_str(), (*_current)[0], (*_current)[1], (*_current)[2]);
	                                 });

	clientCommandsHandler->subscribe("ob_reset", [&](const std::vector<std::string>& args)
	                                 {
		                                 reset();
		                                 CG_Printf("Reseted currently displayed overbounce watcher coordinates.\n");
	                                 });
}

ETJump::OverbounceWatcher::~OverbounceWatcher()
{
	_clientCommandsHandler->unsubcribe("ob_save");
	_clientCommandsHandler->unsubcribe("ob_load");
	_clientCommandsHandler->unsubcribe("ob_reset");
}

void ETJump::OverbounceWatcher::render() const
{
	if (!etj_drawObWatcher.integer)
	{
		return;
	}

	if (!_current)
	{
		return;
	}

	auto ps = getPlayerState();
	if (ps->groundEntityNum != ENTITYNUM_NONE)
	{
		return;
	}

	float psec = pmove_msec.integer / 1000.f;
	int gravity = ps->gravity;
	float velocity = ps->velocity[2];
	float currentHeight = ps->origin[2] + ps->mins[2];
	float finalHeight = 0;

	Coordinate snap{};
	VectorSet(snap, 0, 0, gravity * psec);
	trap_SnapVector(snap.data());
	auto rintv = snap[2];
	
	finalHeight = (*_current)[2] + ps->mins[2];

	// something was hit and it's a floor
	auto b = false;

	// fall ob
	if (isOverbounce(velocity, currentHeight, finalHeight, rintv, psec, gravity))
	{
		b = qtrue;
	}

	// jump ob
	if (isOverbounce(velocity + 270, currentHeight, finalHeight, rintv, psec, gravity))
	{
		b = qtrue;
	}

	// don't predict sticky ob if there is an ob already or if the sticky
	// ob detection isn't requested
	if (b)
	{
		CG_DrawStringExt(etj_obWatcherX.integer, etj_obWatcherY.integer, "OB", colorWhite, qfalse, qtrue, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
	}
}

void ETJump::OverbounceWatcher::beforeRender()
{
}

void ETJump::OverbounceWatcher::save(const std::string& name, Coordinate coordinate)
{
	auto pos = _positions.find(name);
	if (pos != end(_positions))
	{
		_current = nullptr;
	}

	_positions[name] = coordinate;
	_current = &_positions[name];
}

void ETJump::OverbounceWatcher::reset()
{
	_current = nullptr;
}

bool ETJump::OverbounceWatcher::load(const std::string& name)
{
	auto pos = _positions.find(name);
	if (pos == end(_positions))
	{
		return false;
	}

	_current = &(pos->second);
	return true;
}

std::vector<std::string> ETJump::OverbounceWatcher::list() const
{
	std::vector<std::string> names;
	for (const auto& i : _positions)
	{
		names.push_back(i.first);
	}
	return names;
}

const playerState_t* ETJump::OverbounceWatcher::getPlayerState()
{
	return (cg.snap->ps.clientNum != cg.clientNum)
		       // spectating
		       ? &cg.snap->ps
		       // playing
		       : &cg.predictedPlayerState;
}

bool ETJump::OverbounceWatcher::isOverbounce(float vel, float initHeight,
                                             float finalHeight, float rintv,
                                             float psec, int gravity)
{
	float a, b, c;
	float n1;
	float hn;
	int n;

	a = -psec * rintv / 2;
	b = psec * (vel - gravity * psec / 2 + rintv / 2);
	c = initHeight - finalHeight;
	n1 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);

	n = floor(n1);
	hn = initHeight + psec * n * (vel - gravity * psec / 2 - (n - 1) * rintv / 2);

	if (n && hn < finalHeight + 0.25 && hn > finalHeight)
	{
		return true;
	}
	return false;
}

bool ETJump::OverbounceWatcher::surfaceAllowsOverbounce(trace_t* trace)
{
	if (cg_pmove.shared & BG_LEVEL_NO_OVERBOUNCE)
	{
		return ((trace->surfaceFlags & SURF_OVERBOUNCE) != 0);
	}
	else
	{
		return !((trace->surfaceFlags & SURF_OVERBOUNCE) != 0);
	}
}
