#include "etj_utilities.h"

std::string ETJump::composeShader(const char *name, ShaderStage general, ShaderStages stages)
{
	std::string shader;
	// shader name
	shader = shader + name + " {\n";
	// General Directives
	for (auto &dir : general)
	{
		shader = shader + dir + "\n";
	}
	// Stage Directives
	for (auto &stage : stages)
	{
		shader += "{\n";
		for (auto &dir : stage)
		{
			shader = shader + dir + "\n";
		}
		shader += "}\n";
	}
	shader += "}\n";

	return shader;
}

std::string ETJump::composeShader(const char *name, ShaderStages stages)
{
	return composeShader(name, { "" }, stages);
}
