#ifndef MAP_STATISTICS_H
#define MAP_STATISTICS_H
#include <string>
#include <vector>
#include <random>
#include "etj_imap_queries.h"

class MapStatistics : public ETJump::IMapQueries
{
public:
	MapStatistics();
	~MapStatistics();

	struct MapInformation
	{
		MapInformation() : id(0), secondsPlayed(0), callvoted(0), votesPassed(0), timesPlayed(0), lastPlayed(0), isOnServer(false), changed(true)
		{
		}

		long id;
		std::string name;
		int secondsPlayed;
		int callvoted;
		int votesPassed;
		int timesPlayed;
		int lastPlayed;
		bool isOnServer;
		bool changed;
	};

	const char *randomMap() const;

	bool createDatabase();
	bool loadMaps();
	bool loadFromDatabase();
	void saveNewMaps(std::vector<std::string> newMaps);
	void addNewMaps();
	void setCurrentMap(const std::string currentMap);
	const MapInformation *getCurrentMap() const;
	bool initialize(std::string database, const std::string& currentMap);
	void runFrame(int levelTime);
	void resetFields();
	void saveChanges();
	void increaseCallvoteCount(const char *map_name);
	void increasePassedCount(const char *map_name);
	const MapInformation *getMapInformation(const std::string& mapName);
	std::vector<const MapInformation *> getMostPlayed();
	std::vector<const MapInformation *> getLeastPlayed();
	std::vector<std::string> getMaps();
	const std::vector<std::string> *getCurrentMaps();

	//////////////////////////////////////
	// IMapQueries
	//////////////////////////////////////
	bool mapExists(const std::string& map) const override;
	std::vector<std::string> maps() const override;
	std::string currentMap() const override;
	std::vector<std::string> matches(const std::string& map) const override;
	//////////////////////////////////////

private:
	std::vector<MapInformation> _maps;
	std::vector<std::string>    _currentMaps;
	int                         _previousLevelTime;
	// How many milliseconds have elapsed with atleast 1 player on team
	int _currentMillisecondsPlayed;
	// How many milliseconds have elapsed since the map was changed
	int _currentMillisecondsOnServer;
	// What the current map on the server is
	MapInformation *_currentMap;
	std::string    _databaseName;
	int            _originalSecondsPlayed;
};

#endif