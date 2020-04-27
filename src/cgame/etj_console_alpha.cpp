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

#include <string>
#include <boost/format.hpp>

#include "cg_local.h"
#include "etj_console_alpha.h"
#include "etj_utilities.h"

using namespace ETJump;

ConsoleAlphaHandler::ConsoleAlphaHandler()
{
	auto shader = createBackground();
	trap_R_LoadDynamicShader(shaderName, shader.c_str());
	// once shader is registered, any changes to dynamic shader will have no effect
	trap_R_RegisterShader(shaderName);
	trap_R_RemapShader("console-16bit", shaderName, "0");
}

std::string ConsoleAlphaHandler::createBackground()
{
	if (etj_consoleShader.integer > 0)
	{
		return createTexturedBackground();
	}
	return createSolidBackground();
}

std::string ConsoleAlphaHandler::createTexturedBackground()
{
	auto alphaGen = (boost::format("alphaGen const %f") % etj_consoleAlpha.value).str();
	return composeShader(
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
}

std::string ConsoleAlphaHandler::createSolidBackground()
{
	vec4_t bg;
	parseColorString(etj_consoleColor.string, bg);
	auto alphaGen = (boost::format("alphaGen const %f") % etj_consoleAlpha.value).str();
	auto colorGen = (boost::format("rgbGen const ( %f %f %f )") % bg[0] % bg[1] % bg[2]).str();

	return composeShader(
		shaderName,
		{ "nopicmip" },
		{
			{
				"map $whiteimage",
				"blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA",
				alphaGen.c_str(),
				colorGen.c_str(),
			}	
		}
	);
}

ConsoleAlphaHandler::~ConsoleAlphaHandler()
{
	// unload dynamic shader, so if you do `vid_restart` you won't accidently load it twice
	trap_R_LoadDynamicShader(shaderName, nullptr);
	trap_R_RemapShader("console-16bit", "console-16bit", "0");
}
