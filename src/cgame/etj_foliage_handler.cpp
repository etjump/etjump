#include <string>
#include <boost/format.hpp>

#include "cg_local.h"
#include "etj_foliage_handler.h"
#include "etj_utilities.h"
#include "etj_cvar_update_handler.h"

using namespace ETJump;

FoliageHandler::FoliageHandler()
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
	
	if (etj_removeFoliage.integer)
	{
		turnOffFoliage();
	}

	cvarUpdateHandler->subscribe(&etj_removeFoliage, [&](const vmCvar_t* cvar) {
		cvar->integer ? turnOffFoliage() : turnOnFoliage();
	});
}

void FoliageHandler::turnOnFoliage()
{
	for (auto &foliageShader : foliageShaders)
	{
		trap_R_RemapShader(foliageShader, foliageShader, "0");
	}
}

void FoliageHandler::turnOffFoliage()
{
	for (auto &foliageShader : foliageShaders)
	{
		trap_R_RemapShader(foliageShader, shaderName, "0");
	}
}


FoliageHandler::~FoliageHandler()
{
	trap_R_LoadDynamicShader(shaderName, nullptr);
	turnOnFoliage();
}
