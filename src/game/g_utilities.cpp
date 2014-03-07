#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <string>
#include <vector>

#include "g_utilities.hpp"
#include "g_local.hpp"

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

void C_ConsolePrintTo( gentity_t *target, const char* msg ) {
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

void C_ConsolePrintAll( const char* msg ) {
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

void C_ChatPrintTo( gentity_t *target, const char* msg ) {
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

void C_ChatPrintAll( const char* msg ) {
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

void C_CPTo( gentity_t *target, const char* msg ) {
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

void C_CPAll( const char* msg ) {
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

void C_CPMTo( gentity_t *target, const char* msg ) 
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

void C_CPMAll( const char* msg ) 
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

void C_BPTo( gentity_t *target, const char* msg )
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "bp \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand(target->client->ps.clientNum, toPrint);
    } else {
        G_Printf("%s\n", msg);
    }
}

void C_BPAll( const char* msg )
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
        boost::format toPrint("print \"%s\"");
        if( insertNewLine ) {
            toPrint % (bigTextBuffer + NEWLINE);
            trap_SendServerCommand(ent->client->ps.clientNum, 
                toPrint.str().c_str());
        } else {
            toPrint % bigTextBuffer;
            trap_SendServerCommand(ent->client->ps.clientNum,
                toPrint.str().c_str());
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

Arguments GetSayArgs( int start /*= 0*/ )
{
    int argc = trap_Argc();

    static vector<string> argv;
    argv.clear();

    if(start >= argc)
    {
        return &argv;
    }

    for(int i = start; i < argc; i++) {
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
    (const std::string& name, std::string& err) {
    char errorMsg[MAX_TOKEN_CHARS] = "\0";
    int pids[MAX_CLIENTS];
    gentity_t *player;

    if(ClientNumbersFromString(name.c_str(), pids) != 1) {
        G_MatchOnePlayer(pids, errorMsg, sizeof(errorMsg));
        err = errorMsg;
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

std::string SayArgv( int n )
{
    if(n >= Q_SayArgc())
    {
        return "";
    } else
    {
        char arg[MAX_TOKEN_CHARS] = "\0";
        Q_SayArgv(n, arg, sizeof(arg));

        return arg;
    }
}

static void FS_ReplaceSeparators( char *path ) {
    char    *s;

    for ( s = path ; *s ; s++ ) {
        if ( *s == '/' || *s == '\\' ) {
            *s = PATH_SEP;
        }
    }
}

static char* BuildOSPath( const char* file )
{
    char base[MAX_CVAR_VALUE_STRING] = "\0";
    char temp[MAX_OSPATH] = "\0";
    char game[MAX_CVAR_VALUE_STRING] = "\0";
    static char ospath[2][MAX_OSPATH] = {"\0", "\0"};
    static int toggle;

    toggle ^= 1;        // flip-flop to allow two returns without clash

    trap_Cvar_VariableStringBuffer("fs_game", game, sizeof(game));
    trap_Cvar_VariableStringBuffer("fs_homepath", base, sizeof(base));

    Com_sprintf( temp, sizeof( temp ), "/%s/%s", game, file );
    FS_ReplaceSeparators( temp );
    Com_sprintf( ospath[toggle], sizeof( ospath[0] ), "%s%s", base, temp );

    return ospath[toggle];
}

std::string GetPath( const std::string& file )
{
    char *ospath = BuildOSPath( file.c_str() );

    if(!ospath)
    {
        return std::string();
    } else
    {
        return ospath;
    }
}

bool MapExists( const std::string& map )
{
    std::string mapName = "maps/" + map + ".bsp";

    fileHandle_t f = 0;
    trap_FS_FOpenFile(mapName.c_str(), &f, FS_READ);
    trap_FS_FCloseFile(f);

    if(!f)
    {
        return false;
    }
    return true;
}

std::string ValueForKey( gentity_t *ent, const std::string& key )
{
    char userinfo[MAX_INFO_STRING] = "\0";
    char *value = NULL;

    trap_GetUserinfo(ent->client->ps.clientNum, userinfo, sizeof(userinfo));
    value = Info_ValueForKey(userinfo, key.c_str());
    if(!value)
    {
        return "";
    } 

    return value;
}

std::string TimeStampToString( int t )
{
    char buf[MAX_TOKEN_CHARS];
    struct tm *lt = NULL;
    time_t toConvert = t;
    lt = localtime(&toConvert);
    // day / month / year 
    strftime(buf, sizeof(buf), "%d/%m/%y %H:%M:%S", lt);
    return std::string(buf);
}