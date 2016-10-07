#pragma once
#include <string>

namespace ETJump
{
	class ICvarManager
	{
	public:
		virtual ~ICvarManager()
		{
		}

		/**
		 * Returns a matching cvar's string value or empty string
		 */
		virtual std::string getString(const std::string& cvar) = 0;
		/**
		 * Returns a matching cvar's integer value or 0
		 */
		virtual int getInteger(const std::string& cvar) = 0;
	};
}
