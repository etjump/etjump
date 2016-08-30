#pragma once
#include <vector>
#include <string>
#include "etj_imap_queries.h"

namespace ETJump
{
	class MapStats : public IMapQueries
	{
	public:
		explicit MapStats(const std::string& database, const std::string& currentMap);
		~MapStats();
		//////////////
		// IMapQueries
		//////////////
		bool mapExists(const std::string& map) const override;
		std::vector<std::string> maps() const override;
		std::string currentMap() const override;
		std::vector<std::string> matches(const std::string& map) const override;
		//////////////
	private:
		// creates the database if it does not exist
		void createDatabase();

		// get's a list of maps
		static std::vector<std::string> getCurrentMaps();

		// checks if there are any new maps that are not in the database
		// and adds them to the database
		void insertNewMaps();

		std::string _database;
		std::string _currentMapName;
		std::vector<std::string> _currentMapsOnServer;
	};
}
