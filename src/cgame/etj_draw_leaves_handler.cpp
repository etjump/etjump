#include <string>
#include <boost/format.hpp>

#include "cg_local.h"
#include "etj_draw_leaves_handler.h"
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"

using namespace ETJump;

DrawLeavesHandler::DrawLeavesHandler()
{
	auto shader = composeShader(
		shaderName,
	{
		{
		    "map *white",
		    "blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA",
		    "alphaGen const 0.0"
		}
	}
		);

	trap_R_LoadDynamicShader(shaderName, shader.c_str());
	trap_R_RegisterShader(shaderName);

	if (!etj_drawLeaves.integer)
	{
		turnOffLeaves();
	}

	cvarUpdateHandler->subscribe(&etj_drawLeaves, [&](const vmCvar_t *cvar) {
		cvar->integer ? turnOnLeaves() : turnOffLeaves();
	});
}

void DrawLeavesHandler::turnOnLeaves()
{
	for (auto &leavesShader : leavesShaders)
	{
		trap_R_RemapShader(leavesShader, leavesShader, "0");
	}
}

void DrawLeavesHandler::turnOffLeaves()
{
	for (auto &leavesShader : leavesShaders)
	{
		trap_R_RemapShader(leavesShader, shaderName, "0");
	}
}


DrawLeavesHandler::~DrawLeavesHandler()
{
	trap_R_LoadDynamicShader(shaderName, nullptr);
	turnOnLeaves();
}
