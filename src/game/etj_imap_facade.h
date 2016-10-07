#pragma once

/**
 * Interface for querying map related information
 */

#include <vector>

namespace ETJump
{
	class IMapFacade
	{
	public:
		virtual ~IMapFacade()
		{
		}

		// queries
		virtual bool mapExists(const std::string& map) const = 0;
		virtual std::vector<std::string> maps() const = 0;
		virtual std::string currentMap() const = 0;
		// finds all matching maps based on the map name
		virtual std::vector<std::string> matches(const std::string& map) const = 0;

		// actions
		virtual void changeMap(const std::string& newMap) = 0;
	};
}
