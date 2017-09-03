#pragma once
#include <string>

namespace ETJump
{
	struct OperationResult
	{
		OperationResult(const bool success, const std::string& message)
			: success(success),
			message(message)
		{
		}

		const bool success;
		const std::string message;
	};
}
