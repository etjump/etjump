#pragma once
#include <string>
#include <vector>

namespace ETJump
{
	class IMapQueries
	{
	public:
		virtual ~IMapQueries()
		{
		}

		virtual bool mapExists(const std::string& map) const = 0;
		virtual std::vector<std::string> maps() const = 0;
		virtual std::string currentMap() const = 0;
		// finds all matching maps based on the map name
		virtual std::vector<std::string> matches(const std::string& map) const = 0;
	};
}
