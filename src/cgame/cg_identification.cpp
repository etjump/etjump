extern "C" {
#include "cg_local.h"
}

#include <string>
#include <fstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

static std::string user_guid;

std::string GenerateUUID() {
    boost::uuids::random_generator gen;
    boost::uuids::uuid u = gen();

    return boost::uuids::to_string(u);
}

std::string G_SHA1(const std::string& str) {
    char s[MAX_TOKEN_CHARS];
    Q_strncpyz(s, str.c_str(), sizeof(s));
    return std::string(G_SHA1(s));
}

void GenerateGuid() {

    if(user_guid.length() > 0) {
        CG_Printf("^1Error: ^7trying to generate a new guid with an existing user guid\n");
        return;
    }

    char folder_name[MAX_TOKEN_CHARS];
    std::string file_name;

    trap_Cvar_VariableStringBuffer("fs_game", folder_name, sizeof(folder_name));

    if(strlen(folder_name) == 0) {
        file_name = "etjump/etguid";
    } else {
        file_name = std::string(folder_name) + "/etguid";
    }

    std::string etguid = G_SHA1( GenerateUUID() );

    std::ofstream etguid_file(file_name.c_str());

    if(!etguid_file) {
        // Just use the random guid but don't save it anywhere
        user_guid = etguid;
        CG_Printf("^1Error:^7 failed to save guid to a file\n");
        return;
    }

    // Write the guid to the file
    etguid_file << etguid;
    
    etguid_file.close();

    user_guid = etguid;
}

bool ReadGuid() {

    char folder_name[MAX_TOKEN_CHARS];
    std::string file_name;

    trap_Cvar_VariableStringBuffer("fs_game", folder_name, sizeof(folder_name));

    if(strlen(folder_name) == 0) {
        file_name = "etjump/etguid";
    } else {
        file_name = std::string(folder_name) + "/etguid";
    }

    std::ifstream etguid_file(file_name.c_str());

    if(!etguid_file) {
        // Couldn't find the file, create it
        GenerateGuid();
        return true;
    }

    std::string etguid;
    std::getline(etguid_file, etguid);

    if(etguid.length() != 40) {
        GenerateGuid();
        return true;
    }

    user_guid = etguid;
    return true;
}

void SendGuid() {
   ReadGuid();

   trap_SendClientCommand(std::string("etguid " + user_guid).c_str());
}

#if defined __linux__

void CG_Minimize_f(void) {
	return;
}

#elif defined WIN32
#define Rectangle LCC_Rectangle
#include <Windows.h>
#undef Rectangle

// Zero: this doesn't belong here, but I didn't feel like including <Windows.h> again
// just for this 

void CG_Minimize_f ( void ) 
{
	HWND wnd;
	if(wnd = GetForegroundWindow())
	{
		ShowWindow(wnd, SW_MINIMIZE);
	}
}

#endif