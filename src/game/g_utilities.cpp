extern "C" {
#include "g_local.h"
}

#include <sstream>
#include <vector>
#include <string>

using std::vector;
using std::string;

vector<string> GetArgs() {
    vector<string> argv;
    for(int i = 0; i < trap_Argc(); i++) {
        char arg[MAX_TOKEN_CHARS];

        trap_Argv(i, arg, sizeof(arg));
        argv.push_back(arg);
    }
    return argv;
}

std::string G_SHA1(const std::string& str) {
    char s[MAX_TOKEN_CHARS];
    Q_strncpyz(s, str.c_str(), sizeof(s));
    return std::string(G_SHA1(s));
}

string int2string(int value) {
    return std::stringstream(value).str();
}

bool string2int(const string& s, int& i) {
	std::istringstream iss(s);
	if((iss >> i).fail()) {
		return false;
	}	

	return true;
}

void LogPrintln(string msg) {
    if(msg.length() > 1000) {
        msg.resize(1000);
    }
    G_LogPrintf("%s\n", msg.c_str());
}

void LogPrint(string msg) {
    // Just an arbitrary number that won't exceed the 1024 char limit
    if(msg.length() > 1000) {
        msg.resize(1000);
    }

    G_LogPrintf("%s", msg.c_str());
}