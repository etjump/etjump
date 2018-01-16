#ifndef UTILITIES_HH
#define UTILITIES_HH
#include <string>

class ClientUtilities
{
public:
	ClientUtilities() = delete;
	~ClientUtilities() = delete;
	// Duplication, look at game/Utilities.cpp
	// Should refactor these..
	// Returns the actual path (e.g. etjump/folder/file.ext)
	static std::string getPath(const std::string& fileName);
};

#endif