#include "g_sessiondb.hpp"
#include "g_utilities.hpp"
#include "g_local.hpp"
#include "g_database.hpp"

void Reset(Client& toReset)
{
	toReset.guid.clear();
}


void SessionDB::Reset()
{
	for(size_t i = 0; i < MAX_CLIENTS; i++)
	{
		::Reset(clients_[i]);
	}
}

void SessionDB::ResetClient( gentity_t *ent )
{
	if(ent->client) {
		::Reset(clients_[ent->client->ps.clientNum]);
	}
}

std::string SessionDB::Guid( gentity_t *ent )
{
	return clients_[ent->client->ps.clientNum].guid;
}

void SessionDB::Set( gentity_t *ent, const std::string& guid, int level, 
				   const std::string& name, 
				   const std::string& personalCommands, 
				   const std::string& personalGreeting, 
				   const std::string& personalTitle )
{
	clients_[ent->client->ps.clientNum].noguid = false;
	clients_[ent->client->ps.clientNum].guid=guid;
	// Find user level data
    ConstLevelIterator it;
    if(!adminDB.GetLevel(level, it))
    {
        // Make sure it always exists. This shouldn't happen as it just 
        // returns a level 0 one if it does not exist
        G_LogPrintf("Error while trying to get level data from persistant"
            " database. Level: %d.\n", level);
        return;
    }

	// If target has a personal title or greeting, just set them 
	// to the personal ones instead of the level ones. We only need to do
	// this when level/personal data is modified
	if(personalTitle.length() == 0)
	{
		clients_[ent->client->ps.clientNum].title = it->get()->name;
	}
	else
	{
		clients_[ent->client->ps.clientNum].title = personalTitle;
	}

	if(personalGreeting.length() == 0)
	{
		clients_[ent->client->ps.clientNum].greeting = it->get()->greeting;
	}
	else
	{
		clients_[ent->client->ps.clientNum].greeting = personalGreeting;
	}

	// Set client permissions based on the personal&level data
	SetPermissions(clients_[ent->client->ps.clientNum], personalCommands,
		it->get()->commands);

	clients_[ent->client->ps.clientNum].level=level;
}

void ParsePermissions(std::bitset<MAX_COMMANDS>& temp, 
					  const std::string& permissions)
{
	const enum {
		ALLOW,
		DENY
	};

	int state = ALLOW;

	for(size_t i = 0; i < permissions.size(); i++)
	{
		char c = permissions.at(i);
		if(state == ALLOW)
		{
			if(c == '*')
			{
				// Allow all commands
				for(size_t i = 0; i < MAX_COMMANDS; i++)
				{
					temp.set(i, true);
				}
			}
			else if (c == '+')
			{
				// ignore +
				continue;
			}
			else if (c == '-')
			{
				state = DENY;
			}
			else
			{
				temp.set(c, true);
			}
		}
		else
		{
			if(c == '*')
			{
				// Ignore * while in deny-mode
				continue;
			}
			else if(c == '+')
			{
				state = ALLOW;
			}
			else
			{
				temp.set(c, false);
			}
		}
	}
}

void SessionDB::SetPermissions( Client& client, 
							   const std::string& personalCommands, 
							   const std::string& levelCommands )
{
	// Make sure it's empty
	client.permissions.reset();

	// Set permissions based on level commands
	ParsePermissions(client.permissions, levelCommands);

	// Override level perms with personal permissions
	ParsePermissions(client.permissions, personalCommands);
}

std::string SessionDB::Greeting( gentity_t *ent )
{
	return clients_[ent->client->ps.clientNum].greeting;
}

int SessionDB::Level( gentity_t *ent )
{
    return clients_[ent->client->ps.clientNum].level;
}

Client::Client()
{
	this->noguid = true;
	this->guid.clear();
	this->level = 0;
	this->permissions.reset();
	this->greeting.clear();
	this->title.clear();
}
