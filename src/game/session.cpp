#include "g_local.hpp"
#include "session.hpp"
#include <boost/shared_array.hpp>

namespace Session
{
    static boost::shared_array<Client> clients;

    Client::Client()
    {
        this->greeting.clear();
        this->guid.clear();
        this->id = 0;   
        this->level = 0;
        this->permissions.reset();
        this->title.clear();
    }

    void InitGame()
    {
        clients = boost::shared_array<Client>(new Client[64]);
    }

    void ShutdownGame()
    {
        // Free the allocated memory
        clients.reset();
    }

    void Reset(Client& toReset)
    {
        toReset.greeting.clear();
        toReset.guid.clear();
        toReset.id = 0;
        toReset.level = 0;
        toReset.permissions.reset();
        toReset.title.clear();
    }

    void ResetClient(gentity_t *ent)
    {
        Reset(clients[ent->client->ps.clientNum]);
    }
}

