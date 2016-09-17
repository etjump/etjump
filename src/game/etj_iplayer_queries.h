#pragma once
#include <string>

namespace ETJump
{
	class IPlayerQueries
	{
	public:
		virtual std::string name(int clientNum) const = 0;
	};
}
