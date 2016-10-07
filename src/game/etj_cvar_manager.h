#pragma once
#include "etj_icvar_manager.h"

namespace ETJump
{
	class CvarManager : public ICvarManager
	{
	public:

		std::string getString(const std::string& cvar) override;
		int getInteger(const std::string& cvar) override;
	};
}



