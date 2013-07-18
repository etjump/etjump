#ifndef g_clientdb_h__
#define g_clientdb_h__

#include <string>
#include <bitset>
#include "g_local.hpp"

const unsigned MAX_COMMANDS = 256;

struct Client {
	Client();
	bool noguid;
	int level;
	std::string guid;
	std::string greeting;
	std::bitset<MAX_COMMANDS> permissions;	
	std::string title;
};
// Sets all variables to default values
void Reset(Client& toReset);

class SessionDB {
public:
	// Used to reset all clients' data in the data structure
	void Reset();

	// Used to reset client's data in the data structure
	void ResetClient(gentity_t *ent);

	// Returns client's guid
	std::string Guid(gentity_t *ent);

	// Sets ent's guid to "guid", level to "level", !finger name to "name",
	// personal commands to "personalCommands, personal greeting to
	// "personalGreeting" and personal level title to "personalTitle".
	void Set( gentity_t *ent, const std::string& guid, int level, 
		const std::string& name, 
		const std::string& personalCommands, 
		const std::string& personalGreeting, 
		const std::string& personalTitle );

	// Returns client's greeting
	std::string Greeting(gentity_t *ent);

    bool HasPermission(gentity_t *ent, char flag) const;

    int Level(gentity_t *ent);
private:
	void SetPermissions( Client& client, const std::string& personalCommands, const std::string& levelCommands );
	Client clients_[MAX_CLIENTS];
};

#endif // g_clientdb_h__
