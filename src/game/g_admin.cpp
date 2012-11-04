#include <vector>
#include <string>
#include "g_admindatabase.h"

using std::string;
using std::vector;

std::string int2string(int i);
bool string2int(const string& s, int& i);

static ClientDatabase clientDatabase;
static AdminDatabase adminDatabase;

///////////////////////////////////
// Help functions
///////////////////////////////////

vector<string> GetSayArgs() {
    int argc = Q_SayArgc();
	vector<string> argv;

	for(int i = 0; i < argc; i++) {
		char arg[MAX_TOKEN_CHARS];
        Q_SayArgv(i, arg, sizeof(arg));
		argv.push_back(arg);
	}
	return argv;
}

vector<string> GetArgs() {
	int argc = trap_Argc();
    vector<string> argv;

    for(int i = 0; i < argc; i++) {
        char arg[MAX_TOKEN_CHARS];
        trap_Argv(i, arg, sizeof(arg));
        argv.push_back(arg);
    }
    return argv;
}

void DecolorString(const string& in, string& out) {
	string::size_type i = 0;

	out = in;

	while(i < out.length()) {
		if(out.at(i) == 27 || out.at(i) == '^') {
			if(i + 1 < out.length()) {
				out.erase(i+1, 1);
			}
			out.erase(i, 1);
		}
        i++;
	}
}

vector<gentity_t*> GetMatchingPlayers(string name) {
	vector<gentity_t*> matching_players;

    int client_id = -1;

    if(string2int(name, client_id)) {

        if(client_id < 0 || client_id > level.maxclients) {
            return matching_players;
        }

        gclient_t *client = &level.clients[client_id];

        if(client->pers.connected == CON_DISCONNECTED) {
            return matching_players;
        }

        matching_players.push_back(g_entities + client_id);
        return matching_players;
    }

	for(int i = 0; i < level.numConnectedClients; i++) {
		int id = level.sortedClients[i];
		string clean_name;

		DecolorString((g_entities + id)->client->pers.netname, clean_name);

		if(clean_name.find(name) != string::npos) {
			matching_players.push_back(g_entities + id);
		}
	}

	return matching_players;
}

void ChatPrintTo(gentity_t *ent, string message) {
	if(ent) {
		CP(va("chat \"%s\"", message.c_str()));
	} else {
		G_Printf("%s\n", message.c_str());
	}
}

void ChatPrintAll(string message) {
	AP(va("chat \"%s\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void CPMPrintTo(gentity_t *ent, string message) {
	if(ent) {
		CP(va("cpm \"%s\n\"", message.c_str()));
	} else {
		G_Printf("%s\n", message.c_str());
	}
}

void CPMPrintAll(string message) {
	AP(va("cpm \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void CPPrintTo(gentity_t *ent, string message) {
	if(ent) {
		CP(va("cp \"%s\n\"", message.c_str()));
	} else {
		G_Printf("%s\n", message.c_str());
	}
}

void CPPrintAll(string message) {
	AP(va("cp \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void PrintTo(gentity_t *ent, string message) {
	if(ent) {
		CP(va("print \"%s\n\"", message.c_str()));
	} else {
		G_Printf("%s\n", message.c_str());
	} 
}

void PrintAll(string message) {
	AP(va("print \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

static string strBuffer;

void beginBufferPrint() {
	strBuffer.clear();
}

void finishBufferPrint(gentity_t *ent) {
	PrintTo(ent, strBuffer);
}

void bufferPrint(gentity_t *ent, string str) {
	if(!ent) {
		string str2;
		DecolorString(str, str2);

		if(str2.length() + strBuffer.length() > 239) {
			G_Printf("%s", str.c_str());
			strBuffer.clear();
		}

		strBuffer = str2;
	}

	else {
		if(str.length() + strBuffer.length() >= 1009) {
			CP(va("print \"%s\"", strBuffer.c_str()));
			strBuffer.clear();
		}
		strBuffer = str;
	}
}

std::string G_SHA1(const std::string& str) {
    char s[MAX_TOKEN_CHARS];
    Q_strncpyz(s, str.c_str(), sizeof(s));
    return std::string(G_SHA1(s));
}

////////////////////////////////////
// End of help functions
////////////////////////////////////

////////////////////////////////////
// Guid functions
////////////////////////////////////

// Client sent an invalid guid msg, request a new one
void RequestGuid(gentity_t *ent) {
    trap_SendServerCommand(ent->client->ps.clientNum, "guid_request");
}

// Client sent us his/her etjump guid
void GuidReceived(gentity_t *ent) {
    vector<string> argv = GetArgs();

    if(argv.size() != 2) {
        // Request guid as we didn't receive one
        RequestGuid(ent);
        return;
    }

    if(argv.at(1).size() != 40) {
        // Invalid guid, request another
        RequestGuid(ent);
        return;
    }

    clientDatabase.resetClientData(ent->client->ps.clientNum);

    clientDatabase.setActive(ent->client->ps.clientNum);
    clientDatabase.setGuid(ent->client->ps.clientNum, G_SHA1(argv.at(1)));
}

void ResetClientData(int clientNum) {
    clientDatabase.resetClientData(clientNum);
}





qboolean G_admin_readconfig(gentity_t *ent, int skipargs) {

    if(!adminDatabase.readConfig()) {
        ChatPrintTo(ent, "Readconfig error: " + adminDatabase.error());
        return qfalse;
    }

    ChatPrintTo(ent, va("^3readconfig:^7 loaded %d levels, %d users and %d bans", 
        adminDatabase.levelCount(), adminDatabase.userCount(), adminDatabase.banCount()));

    return qtrue;
}