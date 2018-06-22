#ifndef CUSTOM_MAP_VOTES_HH
#define CUSTOM_MAP_VOTES_HH

#include <vector>
#include <string>

class MapStatistics;

class CustomMapVotes
{
public:
	struct MapType
	{
		std::string type;
		std::string callvoteText;
		std::vector<std::string> mapsOnServer;
		std::vector<std::string> otherMaps;
	};

	struct TypeInfo
	{
		TypeInfo(bool typeExists, const std::string& callvoteText) :
			typeExists(typeExists), callvoteText(callvoteText)
		{

		}
		TypeInfo() : typeExists(false)
		{
		}
		bool typeExists;
		std::string callvoteText;
	};

	CustomMapVotes(MapStatistics *mapStats);
	~CustomMapVotes();
	bool Load();
	TypeInfo GetTypeInfo(const std::string& type) const;
	const std::string RandomMap(const std::string& type);
	bool isValidMap(const std::string &mapName);
	std::string ListTypes() const;
	const std::vector<std::string> *ListInfo(const std::string& name);
	void GenerateVotesFile();
private:
	std::vector<MapType>           customMapVotes_;
	const std::vector<std::string> *_currentMapsOnServer;
	MapStatistics                  *_mapStats;
};

#endif
