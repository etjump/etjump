#pragma once
#include <string>

namespace ETJump
{
	struct OperationResult
	{
		OperationResult(bool successful, const std::string& message): success(successful), message(message) {}
		bool success;
		std::string message;
	};
}
