#pragma once

#include <tuple>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
#include "etj_phasing.h"
#include "cg_local.h"

// init static members
std::vector<ETJump::RemapPair> ETJump::PhaseRemapper::_remapsA = std::vector<RemapPair>();
std::vector<ETJump::RemapPair> ETJump::PhaseRemapper::_remapsB = std::vector<RemapPair>();
bool ETJump::PhaseRemapper::_anyRemaps = false;

void ETJump::PhaseRemapper::initialize()
{
	char* buffer;
	const std::string keyBase = "phaseremap";
	const std::string separator = ";";

	for (auto& ab : { "a", "b" })
	{
		for (int i = 0; i < ETJump::PhaseRemapper::maxRemaps; i++)
		{
			const auto key = keyBase + ab + std::to_string(i + 1);

			// spawnstrings must be in order, most maps dont have any so exit early
			if (!CG_SpawnString(key.c_str(), "", &buffer))
			{
				break;
			}

			const auto value = std::string(buffer);

			std::vector<std::string> shaders;
			std::string container;
			boost::split(shaders, value, boost::is_any_of(separator));

			if (shaders.size() != 2)
			{
				CG_Error("Malformed %s value: %s\n", key.c_str(), buffer);
				continue;
			}

			RemapPair rmp = { shaders.at(0), shaders.at(1) };

			if (!trap_R_RegisterShader(rmp.to.c_str()))
			{
				CG_Error("Failed to register shader: %s\n", rmp.to.c_str());
				continue;
			}

			(ab[0] == 'a' ? _remapsA : _remapsB).push_back(rmp);
		}
	}

	_anyRemaps = _remapsA.size() != 0 || _remapsB.size() != 0;
	_remapsA.shrink_to_fit();
	_remapsB.shrink_to_fit();
}

void ETJump::PhaseRemapper::update(int eFlags)
{
	// do remaps only if there are any, and active state changed from last check
	if (!_anyRemaps)
	{
		return;
	}

	bool a = (eFlags & EF_PHASE_A) == EF_PHASE_A;

	if (_isActiveA != a)
	{
		if (_isActiveA = a)
		{
			// remap
			for (auto& shader : _remapsA)
			{
				trap_R_RemapShader(shader.from.c_str(), shader.to.c_str(), "0");
			}
		}
		else
		{
			// restore
			for (auto& shader : _remapsA)
			{
				trap_R_RemapShader(shader.from.c_str(), shader.from.c_str(), "0");
			}
		}
	}

	bool b = (eFlags & EF_PHASE_B) == EF_PHASE_B;

	if (_isActiveB != b)
	{
		if (_isActiveB = b)
		{
			// remap
			for (auto& shader : _remapsB)
			{
				trap_R_RemapShader(shader.from.c_str(), shader.to.c_str(), "0");
			}
		}
		else
		{
			// restore
			for (auto& shader : _remapsB)
			{
				trap_R_RemapShader(shader.from.c_str(), shader.from.c_str(), "0");
			}
		}
	}
}

ETJump::PhaseRemapper::PhaseRemapper() : _isActiveA(false), _isActiveB(false)
{
}

ETJump::PhaseRemapper::~PhaseRemapper()
{
}
