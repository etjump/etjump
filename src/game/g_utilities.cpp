extern "C" {
#include "g_local.h"
}

#include <sstream>
#include <vector>
#include <string>

using std::vector;
using std::string;

void ChatPrintTo(gentity_t *ent, const string& message) {
	if(ent) {
		CP(va("chat \"%s\"", message.c_str()));
	} else {
		G_Printf("%s\n", message.c_str());
	}
}

void ChatPrintAll(const string& message) {
	AP(va("chat \"%s\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void CPMPrintTo(gentity_t *ent, const string& message) {
	if(ent) {
		CP(va("cpm \"%s\n\"", message.c_str()));
	} else {
		G_Printf("%s\n", message.c_str());
	}
}

void CPMPrintAll(const string& message) {
	AP(va("cpm \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void CPPrintTo(gentity_t *ent, const string& message) {
	if(ent) {
		CP(va("cp \"%s\n\"", message.c_str()));
	} else {
		G_Printf("%s\n", message.c_str());
	}
}

void CPPrintAll(const string& message) {
	AP(va("cp \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
}

void PrintTo(gentity_t *ent, const string& message) {
	if(ent) {
		CP(va("print \"%s\n\"", message.c_str()));
	} else {
		G_Printf("%s\n", message.c_str());
	} 
}

void PrintAll(const string& message) {
	AP(va("print \"%s\n\"", message.c_str()));
	G_Printf("%s\n", message.c_str());
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

vector<string> GetSayArgs() {
    vector<string> argv;
    for(int i = 0; i < Q_SayArgc(); i++) {
        char arg[MAX_TOKEN_CHARS];

        Q_SayArgv(i, arg, sizeof(arg));
        argv.push_back(arg);
    }
    return argv;
}

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
    std::stringstream ss;
    ss << value;
    string s;
    ss >> s;
    return s;
}

bool string2int(const string& s, int& i) {
	std::istringstream iss(s);
	if((iss >> i).fail()) {
		return false;
	}	

	return true;
}

void SanitizeString(const string& in, string& out, bool toLower) {
    string::size_type i = 0;
    while( i < in.size() ) {
        
        if(in[i] == 27 || in[i] == '^') {
            i++;
            if(i < in.size()) {
                continue;
            }
        }

        if(in[i] < 32) {
            i++;
            continue;
        }

        out.push_back(toLower ? tolower(in[i]) : in[i]);
        i++;
    }
}

gentity_t *playerFromName(const string& name, string& error) {
    char err[MAX_STRING_CHARS];
    int pids[MAX_CLIENTS];
    if(ClientNumbersFromString(name.c_str(), pids) != 1) {
        G_MatchOnePlayer(pids, err, sizeof(err));
        error = err;
        return 0;
    }

    return (g_entities + pids[0]);
}

namespace utilities {

    static string bigTextBuffer;

}

void beginBufferPrint() {
    utilities::bigTextBuffer.clear();
}

void finishBufferPrint(gentity_t *ent) {
    PrintTo(ent, utilities::bigTextBuffer);
}

void bufferPrint(gentity_t *ent, const string& msg) {

    if(!ent) {

        if(msg.length() + utilities::bigTextBuffer.length() > 239) {
            G_Printf("%s", utilities::bigTextBuffer.c_str());
            utilities::bigTextBuffer.clear();
        }
        utilities::bigTextBuffer += msg;
    }

    else {
        if(msg.length() + utilities::bigTextBuffer.length() >= 1009) {
            CP( string("print \""+utilities::bigTextBuffer+"\"").c_str() );
            utilities::bigTextBuffer.clear();
        }
        utilities::bigTextBuffer+=msg;
    }

}