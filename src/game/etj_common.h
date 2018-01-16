#pragma once
#include <string>
#include <functional>

namespace ETJump
{
	int subcribeToRunFrame(std::function<void(int)> callback);
	void unsubcribeToRunFrame(int id);
}
