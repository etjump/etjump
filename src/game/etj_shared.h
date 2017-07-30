#pragma once
#include <string>

namespace ETJump
{
	namespace Constants
	{
		namespace Common
		{
			const int MAX_CONNECTED_CLIENTS = 64;
		}

		namespace Authentication
		{
			const int GUID_LEN = 40;
			const std::string GUID_REQUEST = "guid_request";
			const std::string AUTHENTICATE = "authenticate";
		}
	}
}
