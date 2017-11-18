#include <string>
#include <boost/format.hpp>

#include "cg_local.h"
#include "etj_console_alpha.h"
#include "etj_utilities.h"

using namespace ETJump;

ConsoleAlphaHandler::ConsoleAlphaHandler()
{
	auto alphaGen = (boost::format("alphaGen const %f") % etj_consoleAlpha.value).str();
	auto shader = composeShader(
		shaderName,
		{ "nopicmip" },
	{
		{
		    "map textures/skies_sd/wurzburg_clouds.tga",
		    "blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA",
		    alphaGen.c_str(),
		    "rgbGen identityLighting",
		    "tcMod scale 1.5 .75",
		    "tcMod scroll 0.01 -0.005"
		},
		{
		    "map textures/skies_sd/wurzburg_clouds.tga",
		    "blendFunc GL_DST_COLOR GL_ONE",
		    "rgbGen identityLighting",
		    "tcMod scale 1.25 .85",
		    "tcMod transform 0 1 1 0 0 0",
		    "tcMod scroll 0.03 -0.015",
		},
		{
		    "clampmap textures/skies_sd/colditz_mask.tga",
		    "blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA",
		    alphaGen.c_str(),
		    "rgbGen identityLighting",
		    "tcMod scale 0.85 0.5",
		    "tcMod transform 1 0 0 1 -0.075 0.42",
		}
	}
		);

	trap_R_LoadDynamicShader(shaderName, shader.c_str());
	// once shader is registered, any changes to dynamic shader will have no effect
	trap_R_RegisterShader(shaderName);
	trap_R_RemapShader("console-16bit", shaderName, "0");
}

ConsoleAlphaHandler::~ConsoleAlphaHandler()
{
	// unload dynamic shader, so if you do `vid_restart` you won't accidently load it twice
	trap_R_LoadDynamicShader(shaderName, nullptr);
	trap_R_RemapShader("console-16bit", "console-16bit", "0");
}
