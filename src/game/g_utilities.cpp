#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>

#include "g_utilities.h"

using std::string;
using std::vector;

/*
 * Printing related
 */

void ConsolePrintTo( gentity_t *target, const string& msg ) 
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "print \"%s\n\"", msg.c_str());
    if( target ) {
        trap_SendServerCommand( target->client->ps.clientNum, toPrint );
    }
    else {
        G_Printf("%s\n", msg.c_str());
    }
}

void ConsolePrintTo( gentity_t *target, const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "print \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand( target->client->ps.clientNum, toPrint );
    }
    else {
        G_Printf("%s\n", msg);
    }
}

void ConsolePrintAll( const string& msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "print \"%s\n\"", msg.c_str());
    trap_SendServerCommand( -1, toPrint );
    G_Printf("%s\n", msg.c_str());
}

void ConsolePrintAll( const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "print \"%s\n\"", msg);
    trap_SendServerCommand( -1, toPrint );
    G_Printf("%s\n", msg);
}

void ChatPrintTo( gentity_t *target, const string& msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "chat \"%s\"", msg.c_str());
    if( target ) {
        trap_SendServerCommand( target->client->ps.clientNum, toPrint );
    }
    else {
        G_Printf("%s\n", msg.c_str());
    }
}

void ChatPrintTo( gentity_t *target, const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "chat \"%s\"", msg);
    if( target ) {
        trap_SendServerCommand( target->client->ps.clientNum, toPrint );
    }
    else {
        G_Printf("%s\n", msg);
    }
}

void ChatPrintAll( const string& msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "chat \"%s\"", msg.c_str());
    trap_SendServerCommand( -1, toPrint );
    G_Printf("%s\n", msg.c_str());
}

void ChatPrintAll( const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "chat \"%s\"", msg);
    trap_SendServerCommand( -1, toPrint );
    G_Printf("%s\n", msg);
}

void CPTo( gentity_t *target, const string& msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cp \"%s\n\"", msg.c_str());
    if( target ) {
        trap_SendServerCommand( target->client->ps.clientNum, toPrint );
    }
    else {
        G_Printf("%s\n", msg.c_str());
    }
}

void CPTo( gentity_t *target, const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cp \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand( target->client->ps.clientNum, toPrint );
    }
    else {
        G_Printf("%s\n", msg);
    }
}

void CPAll( const string& msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cp \"%s\n\"", msg.c_str());
    trap_SendServerCommand( -1, toPrint );
    G_Printf("%s\n", msg.c_str());
}

void CPAll( const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cp \"%s\n\"", msg);
    trap_SendServerCommand( -1, toPrint );
    G_Printf("%s\n", msg);
}

void CPMTo( gentity_t *target, const string& msg ) 
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cpm \"%s\n\"", msg.c_str());
    if( target ) {
        trap_SendServerCommand(target->client->ps.clientNum, toPrint);
    }
    else {
        G_Printf("%s\n", msg.c_str());
    }
}

void CPMTo( gentity_t *target, const char* msg ) 
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cpm \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand(target->client->ps.clientNum, toPrint);
    }
    else {
        G_Printf("%s\n", msg);
    }
}

void CPMAll( const string& msg ) 
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cpm \"%s\n\"", msg.c_str());
    trap_SendServerCommand(-1, toPrint);
    G_Printf("%s\n", msg.c_str());
}

void CPMAll( const char* msg ) 
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cpm \"%s\n\"", msg);
    trap_SendServerCommand(-1, toPrint);
    G_Printf("%s\n", msg);
}

void BPTo( gentity_t *target, const string& msg )
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "bp \"%s\n\"", msg.c_str());
    if( target ) {
        trap_SendServerCommand(target->client->ps.clientNum, toPrint);
    }
    else { 
        G_Printf("%s\n", msg.c_str());
    }
}

void BPAll( const string& msg )
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "bp \"%s\n\"", msg.c_str());
    trap_SendServerCommand(-1, toPrint);

    G_Printf("%s\n", msg.c_str());
}

void BPTo( gentity_t *target, const char* msg )
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "bp \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand(target->client->ps.clientNum, toPrint);
    } else {
        G_Printf("%s\n", msg);
    }
}

void BPAll( const char* msg )
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "bp \"%s\n\"", msg);
    trap_SendServerCommand(-1, toPrint);
    
    G_Printf("%s\n", msg);
}

static string bigTextBuffer;

void BeginBufferPrint() {
    bigTextBuffer.clear();
}

void FinishBufferPrint(gentity_t *ent, bool insertNewLine) {
    if(ent) {
        if( insertNewLine ) {
            trap_SendServerCommand(ent->client->ps.clientNum, 
                (bigTextBuffer + NEWLINE).c_str());
        } else {
            trap_SendServerCommand(ent->client->ps.clientNum,
                bigTextBuffer.c_str());
        }
    } else {
        if( insertNewLine ) {
            G_Printf("%s\n",
                (bigTextBuffer).c_str());
        } else {
            G_Printf("%s",
                bigTextBuffer.c_str());
        }
    }
}

void BufferPrint( gentity_t *ent, const string& msg ) {

    if(!ent) {
        char cleanMsg[MAX_TOKEN_CHARS];
        SanitizeConstString(msg.c_str(), cleanMsg, qfalse);
        if(cleanMsg) {
            if(strlen(cleanMsg) + bigTextBuffer.length() > 239) {
                G_Printf("%s", bigTextBuffer.c_str());
                bigTextBuffer.clear();
            }
            bigTextBuffer += msg;
        }
    }

    else {
        if( msg.length() + bigTextBuffer.length() > 1009 ) {
            trap_SendServerCommand(ent->client->ps.clientNum, 
                std::string("print \"" + bigTextBuffer + "\"").c_str() );
        }
        bigTextBuffer += msg;
    }
}

/*
 * End of printing related
 */

/*
 * Argument handling
 */

Arguments GetArgs()
{
    int argc = trap_Argc();
    static vector<string> argv;
    argv.clear();

    for(int i = 0; i < argc; i++) {
        char arg[MAX_TOKEN_CHARS];
        trap_Argv(i, arg, sizeof(arg));
        argv.push_back(arg);
    }
    return &argv;
}

Arguments GetSayArgs() {
    int argc = trap_Argc();

    static vector<string> argv;
    argv.clear();

    for(int i = 0; i < argc; i++) {
        char arg[MAX_TOKEN_CHARS];
        trap_Argv(i, arg, sizeof(arg));
        argv.push_back(arg);
    }
    return & argv;
}

/*
 * Conversions
 */

qboolean StringToInt( const char* toConvert, int *value )
{
    try {
        static int result = boost::lexical_cast<int>(toConvert);

        *value = result;
    } 
    catch( /* boost::bad_lexical_cast& e */ ... ) {
        return qfalse;
    }
    return qtrue;
}

bool StringToInt( const string& toConvert, int& value )
{
    try {
        int result = boost::lexical_cast<int>(toConvert);

        value = result;
    } 
    catch( /* boost::bad_lexical_cast& e */ ... ) {
        return false;
    }
    return true;
}

std::string IntToString( int value ) {
    return boost::lexical_cast<std::string>(value);
}

std::string Vec3ToString( vec3_t toConvert ) {
    string vec3 = "(" + IntToString( toConvert[0] )
        + ", " + IntToString( toConvert[1] ) + ", " +
        IntToString( toConvert[2] ) + ")";
    return vec3;
}

std::string Vec3ToString( vec_t x, vec_t y, vec_t z ) {
    string vec3 = "(" + IntToString( x )
        + ", " + IntToString( y ) + ", " +
        IntToString( z ) + ")";
    return vec3;
}

gentity_t *PlayerGentityFromString(char *name, char *err, int size) {
    int pids[MAX_CLIENTS];
    gentity_t *player;

    if(ClientNumbersFromString(name, pids) != 1) {
        G_MatchOnePlayer(pids, err, size);
        return NULL;
    }

    player = g_entities + pids[0];
    return player;
}

gentity_t *PlayerGentityFromString
    (const std::string& name, char *err, int size) {
    int pids[MAX_CLIENTS];
    gentity_t *player;

    if(ClientNumbersFromString(name.c_str(), pids) != 1) {
        G_MatchOnePlayer(pids, err, size);
        return NULL;
    }

    player = g_entities + pids[0];
    return player;
}

void RemoveDuplicates(std::string& in) {
    bool exists[256];
    char buf[MAX_TOKEN_CHARS];
    const char *inPtr = &in[0];
    int count = 0;

    for(int i = 0; i < 256; i++) {
        exists[i] = false;
    }

    while(*inPtr != NULL) {

        if(!exists[*inPtr]) {
            buf[count] = *inPtr;
            count++;
        }

        exists[*inPtr] = true;

        inPtr++;
    }
    buf[count] = 0;
    in = buf;
}

void CharPtrToString( const char* p, std::string& s ) 
{
	if(p) 
	{
		s = p;
	} 
	else
	{
		s = "";
	}
}