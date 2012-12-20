#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

extern "C" {
#include "g_local.h"
}

using std::string;
using std::vector;

/*
==============================
String modification
==============================
*/

// Removes color tags from string

string RemoveColors(const string& source) {
    
    size_t output_length = 0;
    char no_colors[MAX_TOKEN_CHARS] = "\0";
    char *output_ptr = &no_colors[0];
    const char *source_ptr = source.c_str();

    while(*source_ptr) {
        // Return the string if we exceed 
        if(output_length >= MAX_TOKEN_CHARS) {
            return no_colors;
        }

        // If we found ^ in source
        if(*source_ptr == 27 || *source_ptr == '^') {
            // Skip it
            source_ptr++;
            // Skip the next char aswell if it exists
            if(*source_ptr) {
                source_ptr++;
            }
            continue;
        }
        *output_ptr++ = *source_ptr++;
        output_length++;
    }
    *output_ptr = 0;

    return no_colors;
}

// Removes color codes & converts to lower if required
void SanitizeString(const string& in, string& out, bool to_lower) {
    out = RemoveColors(in);

    if(to_lower) {
        boost::to_lower(out);
    }

}

/*
======================
Printing
======================
*/

// Prints text in chat to target entity only
// if entity is null prints to console

void ChatPrintTo(gentity_t *ent, const string& message) {

    if(ent) {
        CP(va("chat \"%s\"", message.c_str()));
    }

    else {
        G_Printf("%s\n", RemoveColors(message).c_str());
    }

}

// cpm to ent
void CPMPrintTo(gentity_t *ent, const string& message) {
    if(ent) {
        CP(va("cpm \"%s\n\"", message.c_str()));
    }

    else {
        G_Printf("%s\n", RemoveColors(message).c_str());
    }
}

// cp to ent
void CPPrintTo(gentity_t *ent, const string& message) {
    if(ent) {
        CP(va("cp \"%s\n\"", message.c_str()));
    }

    else {
        G_Printf("%s\n", RemoveColors(message).c_str());
    }
}

void PrintTo(gentity_t *ent, const string& message) {
    if(ent) {
        CP(va("print \"%s\n\"", message.c_str()));
    }

    else {
        G_Printf("%s\n", RemoveColors(message).c_str());
    }
}

// Prints text in chat to all entities

void ChatPrintAll(const string& message) {
    AP(va("chat \"%s\"", message.c_str()));
    G_Printf("%s\n", RemoveColors(message).c_str());
}

void CPMPrintAll(const string& message) {
    AP(va("cpm \"%s\n\"", message.c_str()));
    G_Printf("%s\n", RemoveColors(message).c_str());
}

void CPPrintAll(const string& message) {
    AP(va("cp \"%s\n\"", message.c_str()));
    G_Printf("%s\n", RemoveColors(message).c_str());
}

void PrintAll(const string& message) {
    AP(va("print \"%s\n\"", message.c_str()));
    G_Printf("%s\n", RemoveColors(message).c_str());
}

void BannerPrintAll(const string& message) {
    AP(va("bp \"%s\n\"", message.c_str()));
    G_Printf("%s\n", RemoveColors(message).c_str());
}

// LogPrint that includes newline
void LogPrintln(const string& message) {
    G_LogPrintf("%s\n", message.c_str());
}

void LogPrint(const string& message) {
    G_LogPrintf("%s", message.c_str());
}

// Buffer printing for bigger than 1024 char string

static string big_text_buffer;

void BeginBufferPrint() {
    big_text_buffer.clear();
}

void FinishBufferPrint(gentity_t *ent) {
    PrintTo(ent, big_text_buffer);
}

void BufferPrint(gentity_t *ent, const string& message) {

    if(!ent) {
        // No need to print color codes to console
        string clean_message = RemoveColors(message);
        
        if(clean_message.length() + big_text_buffer.length() > 239) {
            G_Printf("%s", big_text_buffer.c_str());
            big_text_buffer.clear();
        }
        big_text_buffer += message;
    }

    else {

        if(message.length() + big_text_buffer.length() > 1009) {
            CP( string("print \"" + big_text_buffer + "\"").c_str() );
            big_text_buffer.clear();
        }
        big_text_buffer += message;
    }

}
/*
==================
Argument handling
==================
*/

// Parses arguments for admin system
// Example: say "!kick a" -> "say" "!kick" "a" instead of say & kick a
vector<string> GetSayArgs() {
    vector<string> argv;

    for(int i = 0; i < Q_SayArgc(); i++) {
        char arg[MAX_TOKEN_CHARS];

        Q_SayArgv(i, arg, sizeof(arg));
        argv.push_back(arg);
    }
    return argv;
}

// Gets all arguments
vector<string> GetArgs() {
    vector<string> argv;
    for(int i = 0; i < trap_Argc(); i++) {
        char arg[MAX_TOKEN_CHARS];

        trap_Argv(i, arg, sizeof(arg));
        argv.push_back(arg);
    }
    return argv;
}

/*
==================
Conversions
==================
*/

// Returns an empty string if it fails
string IntToString(int to_convert) {
    string target;
    try {
        target = boost::lexical_cast<std::string, int>(to_convert);
    } catch( ... ) {
        // Should never happen
        return "";
    }
    return target;
}

// Returns true if conversion succeeded
bool StringToInt(const string& source, int& target) {
    try {
        target = boost::lexical_cast<int, std::string>(source);
    } catch ( boost::bad_lexical_cast ) {
        return false;
    }
	return true;
}

// Other utilities

// Sha1 hashing for a string, max len 1024
string SHA1(const string& to_hash) {
    char temp[MAX_TOKEN_CHARS];

    Q_strncpyz(temp, to_hash.c_str(), sizeof(temp));
    return G_SHA1(temp);
}

// returns pointer to player 
gentity_t *PlayerForName(const string& name, string& error) {
    char err[MAX_STRING_CHARS];
    int pids[MAX_CLIENTS];
    if(ClientNumbersFromString(name.c_str(), pids) != 1) {
        G_MatchOnePlayer(pids, err, sizeof(err));
        error = err;
        return 0;
    }

    return (g_entities + pids[0]);
}