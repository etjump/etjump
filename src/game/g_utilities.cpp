#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <regex>
#include <vector>

#include "utilities.hpp"
#include "g_local.hpp"

using std::string;
using std::vector;

//
// Notes
// Max "chat" length is 150
// Max client "print" length is 998
// Max server "print" length is 1013

const int MAX_CHAT_LEN = 150;
const int MAX_CLIENT_PRINT_LEN = 998;
const int MAX_SERVER_PRINT_LEN = 1013;

void ConsolePrintTo(gentity_t *target, boost::format fmt)
{
    int maxLen = MAX_CLIENT_PRINT_LEN;
    std::string message = fmt.str();

    if (!target)
    {
        maxLen = MAX_SERVER_PRINT_LEN;
    }

    while (message.length() > 0)
    {
        if (message.length() > maxLen)
        {
            string toPrint = message.substr(0, maxLen);
            message = message.substr(maxLen + 1);
            if (!target)
            {
                G_Printf("%s", toPrint.c_str());
            }
            else
            {
                trap_SendServerCommand(ClientNum(target), va("print \"%s\"", toPrint.c_str()));
            }
        }
        else
        {
            if (!target)
            {
                G_Printf("%s", message.c_str());
            }
            else
            {
                trap_SendServerCommand(ClientNum(target), va("print \"%s\"", message.c_str()));
            }
            return;
        }
    }
}

void LogPrint(boost::format fmt)
{
    string message = fmt.str();
    while (message.length() > 0)
    {
        if (message.length() > MAX_SERVER_PRINT_LEN)
        {
            string toPrint = message.substr(0, MAX_SERVER_PRINT_LEN);
            message = message.substr(MAX_SERVER_PRINT_LEN + 1);
            G_LogPrintf("%s", toPrint.c_str());
        }
        else
        {
            G_LogPrintf("%s", message.c_str());
        }
    }
}

/*
 * Printing related
 */
void ConsolePrintTo( gentity_t *target, const string& msg ) 
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "print \"%s\n\"", msg.c_str());
    if( target ) {
        trap_SendServerCommand(ClientNum(target), toPrint);
    }
    else {
        G_Printf("%s\n", msg.c_str());
    }
}

void C_ConsolePrintTo( gentity_t *target, const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "print \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand(ClientNum(target), toPrint);
    }
    else {
        G_Printf("%s\n", msg);
    }
}

void ConsolePrintAll(const string& msg, bool toConsole) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "print \"%s\n\"", msg.c_str());
    trap_SendServerCommand( -1, toPrint );
    if (toConsole)
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
        trap_SendServerCommand( ClientNum(target), toPrint );
    }
    else {
        G_Printf("%s\n", msg.c_str());
    }
}

void C_ChatPrintTo( gentity_t *target, const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "chat \"%s\"", msg);
    if( target ) {
        trap_SendServerCommand(ClientNum(target), toPrint);
    }
    else {
        G_Printf("%s\n", msg);
    }
}

void ChatPrintAll(const string& msg, bool toConsole) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "chat \"%s\"", msg.c_str());
    trap_SendServerCommand( -1, toPrint );
    if (toConsole)
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
        trap_SendServerCommand(ClientNum(target), toPrint);
    }
    else {
        G_Printf("%s\n", msg.c_str());
    }
}

void C_CPTo( gentity_t *target, const char* msg ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cp \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand(ClientNum(target), toPrint);
    }
    else {
        G_Printf("%s\n", msg);
    }
}

void CPAll( const string& msg, bool toConsole ) {
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cp \"%s\n\"", msg.c_str());
    trap_SendServerCommand( -1, toPrint );
    if (toConsole)
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
        trap_SendServerCommand(ClientNum(target), toPrint);
    }
    else {
        G_Printf("%s\n", msg.c_str());
    }
}

void CPMTo(gentity_t *target, boost::format fmt)
{
    std::string toPrint = "cpm \"" + fmt.str() + "\n\"";
    if (target)
    {
        trap_SendServerCommand(ClientNum(target), toPrint.c_str());
    }
    else
    {
        G_Printf("%s\n", fmt.str().c_str());
    }
}

void C_CPMTo( gentity_t *target, const char* msg ) 
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cpm \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand(ClientNum(target), toPrint);
    }
    else {
        G_Printf("%s\n", msg);
    }
}

void CPMAll( const string& msg, bool toConsole ) 
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "cpm \"%s\n\"", msg.c_str());
    trap_SendServerCommand(-1, toPrint);
    if (toConsole)
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
        trap_SendServerCommand(ClientNum(target), toPrint);
    }
    else { 
        G_Printf("%s\n", msg.c_str());
    }
}

void LogPrint(std::string message)
{
    G_LogPrintf("%s\n", message.c_str());
}

void BPAll(const string& msg, bool toConsole)
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "bp \"%s\n\"", msg.c_str());
    trap_SendServerCommand(-1, toPrint);

    if (toConsole)
        G_Printf("%s\n", msg.c_str());
}

void C_BPTo( gentity_t *target, const char* msg )
{
    char toPrint[MAX_TOKEN_CHARS] = "\0";
    Com_sprintf(toPrint, sizeof(toPrint), "bp \"%s\n\"", msg);
    if( target ) {
        trap_SendServerCommand(ClientNum(target), toPrint);
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
            trap_SendServerCommand(ClientNum(ent),
                toPrint.str().c_str());
        } else {
            toPrint % bigTextBuffer;
            trap_SendServerCommand(ClientNum(ent),
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
            std::string toSend = std::string("print \"" + bigTextBuffer + "\"");
            trap_SendServerCommand(ClientNum(ent),
                toSend.c_str() );
            bigTextBuffer.clear();
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
    int argc = Q_SayArgc();

    static vector<string> argv;
    argv.clear();

    if(start >= argc)
    {
        return &argv;
    }

    for(int i = start; i < argc; i++) {
        char arg[MAX_TOKEN_CHARS];
        Q_SayArgv(i, arg, sizeof(arg));
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

bool ToInt( const string& toConvert, int& value )
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

bool ToUnsigned(const std::string& toConvert, unsigned& value)
{
    try {
        unsigned result = boost::lexical_cast<unsigned>(toConvert);

        value = result;
    }
    catch ( /* boost::bad_lexical_cast& e */ ...) {
        return false;
    }
    return true;
}


bool ToFloat(const std::string& toConvert, float& value)
{
    try {
        float result = boost::lexical_cast<float>(toConvert);

        value = result;
    }
    catch ( /* boost::bad_lexical_cast& e */ ...) {
        return false;
    }
    return true;
}

std::string ToString( int value ) {
    return boost::lexical_cast<std::string>(value);
}

std::string LongToString(long value) {
    return boost::lexical_cast<std::string>(value);
}

std::string ToString( vec3_t toConvert ) {
    string vec3 = "(" + ToString( toConvert[0] )
        + ", " + ToString( toConvert[1] ) + ", " +
        ToString( toConvert[2] ) + ")";
    return vec3;
}

std::string ToString( vec_t x, vec_t y, vec_t z ) {
    string vec3 = "(" + ToString( x )
        + ", " + ToString( y ) + ", " +
        ToString( z ) + ")";
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

string GetPath( const std::string& file )
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
    string mapName = "maps/" + map + ".bsp";

    fileHandle_t f = 0;
    trap_FS_FOpenFile(mapName.c_str(), &f, FS_READ);
    trap_FS_FCloseFile(f);

    if(!f)
    {
        return false;
    }
    return true;
}

qboolean G_MapExists(const char *map)
{
    if (!map)
    {
        G_Error("map is NULL.");
    }

    return MapExists(map) ? qtrue : qfalse;
}

string ValueForKey(gentity_t *ent, const std::string& key)
{
    char userinfo[MAX_INFO_STRING] = "\0";
    char *value = NULL;

    trap_GetUserinfo(ClientNum(ent), userinfo, sizeof(userinfo));
    value = Info_ValueForKey(userinfo, key.c_str());
    if (!value)
    {
        return "";
    }

    return value;
}

string ValueForKey(int clientNum, const std::string& key)
{
    char userinfo[MAX_INFO_STRING] = "\0";
    char *value = NULL;

    trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
    value = Info_ValueForKey(userinfo, key.c_str());
    if (!value)
    {
        return "";
    }

    return value;
}

string TimeStampToString( int t )
{
    char buf[MAX_TOKEN_CHARS];
    struct tm *lt = NULL;
    time_t toConvert = t;
    lt = localtime(&toConvert);
    if(t > 0)
    {
        // day / month / year 
        strftime(buf, sizeof(buf), "%d/%m/%y %H:%M:%S", lt);
    } else
    {
        return "never";
    }
    
    return std::string(buf);
}

std::string TimeStampDifferenceToString(int diff)
{
    const int MINUTE = 60;
    const int HOUR = 60 * MINUTE;
    const int DAY = 24 * HOUR;
    const int WEEK = 7 * DAY;
    const int MONTH = 30 * DAY;
    const int YEAR = 365 * DAY;

    if(diff < HOUR)
    {
        return ToString(diff/MINUTE) + " minutes";
    } else if(diff < DAY)
    {
        return ToString(diff/HOUR) + " hours";
    } else if(diff < WEEK)
    {
        return ToString(diff/DAY) + " days";
    } else if(diff >= WEEK && diff < MONTH)
    {
        return ToString(diff/WEEK) + " weeks";
    } else if(diff >= MONTH && diff < YEAR)
    {
        return ToString(diff/MONTH) + " months";
    } else if(diff >= YEAR)
    {
        return ToString(diff/YEAR) + " years";
    }
    return "";
}

bool ValidGuid(std::string guid)
{
    const unsigned GUID_SIZE = 40;
    if (guid.size() != GUID_SIZE)
    {
        return false;
    }

    for (size_t i = 0; i < guid.size(); i++)
    {
        if (guid[i] < '0' || guid[i] > 'F')
        {
            return false;
        }
    }
    return true;
}

const char *EscapeString(const char *in)
{
    string str = in;
    boost::replace_all(str, "=", "\x19=");
    static char out[MAX_TOKEN_CHARS] = "\0";
    Q_strncpyz(out, str.c_str(), sizeof(out));
    return out;
}

std::string SanitizeConstString(const std::string& s, bool toLower)
{
    char sanitized[MAX_TOKEN_CHARS];
    SanitizeConstString(s.c_str(), sanitized, toLower ? qtrue : qfalse);
    return sanitized;
}

BufferPrinter::BufferPrinter(gentity_t* ent) : ent_(ent), buffer_("")
{
}

void BufferPrinter::Begin()
{
    buffer_.clear();
}

void BufferPrinter::Print(std::string const& data)
{
    if (!ent_)
    {
        std::string sanitized = SanitizeConstString(data, qfalse);
        
        if (sanitized.length() + buffer_.length() > 239)
        {
            G_Printf("%s", buffer_.c_str());
            buffer_.clear();
        }
        buffer_ += data;
    }
    else
    {
        if (data.length() + buffer_.length() > 1009)
        {
            std::string toSend = std::string("print \"" + buffer_ + "\"");
            trap_SendServerCommand(ClientNum(ent_), toSend.c_str());
            buffer_.clear();
        } 
        buffer_ += data;
    }
}

void BufferPrinter::Finish(bool insertNewLine)
{
    if (ent_)
    {
        boost::format toPrint("print \"%s\"");
        if (insertNewLine)
        {
            toPrint % (buffer_ + NEWLINE);
            trap_SendServerCommand(ClientNum(ent_), toPrint.str().c_str());
        }
        else
        {
            toPrint % buffer_;
            trap_SendServerCommand(ClientNum(ent_), toPrint.str().c_str());
        }
    }
    else
    {
        if (insertNewLine)
        {
            G_Printf("%s\n",
                buffer_.c_str());
        }
        else
        {
            G_Printf("%s", buffer_.c_str());
        }
    }
}

std::vector<std::string> getNames(const std::vector<int>& ids)
{
	std::vector<std::string> names;

	for (auto &id : ids) {
		names.push_back((g_entities + id)->client->pers.netname);
	}

	return std::move(names);
}

std::vector<int> getMatchingIds(const std::string& name)
{
	int pids[MAX_CLIENTS];
	auto found = ClientNumbersFromString(name.c_str(), pids);
	std::vector<int> pidsVector;
	if (found)
	{
		for (auto i = 0; pids[i] != -1; i++)
		{
			pidsVector.push_back(pids[i]);
		}
	}	
	return std::move(pidsVector);
}

// TODO:
// This could be done by simply
// splitting input by @
// and looping
// for i = 1, len = split.len - 1; i < len; i += 2
//   split[i] <- tag
std::string interpolateNametags(std::string input)
{
	std::string interpolated;
	std::regex tagRegex("@(.*?)@");

	auto inputBegin = std::begin(input);
	auto inputEnd = std::end(input);

	std::smatch match;
	size_t begin = 0;
	for (auto iter = std::sregex_iterator(inputBegin, inputEnd, tagRegex); iter != std::sregex_iterator(); ++iter)
	{
		match = *iter;

		interpolated += input.substr(begin, match.position() - begin);	
		auto tag = match.str().substr(1, match.str().length() - 2);
		
		auto names = getNames(getMatchingIds(tag));
		if (names.size() > 0)
		{
			auto joined = boost::algorithm::join(names, "^2, ^7");
			interpolated += + "^7" + joined + "^2";
		} else
		{
			interpolated += match.str();
		}
		
		begin = match.position() + match.length();
	}

	if (begin < input.length())
	{
		interpolated += input.substr(begin);
	}
	return interpolated;
}

const char *interpolateNametags(const char *text)
{
	static char buf[MAX_SAY_TEXT] = "\0";

	auto interpolated = interpolateNametags(std::string(text));

	Q_strncpyz(buf, interpolated.c_str(), sizeof(buf));
	return buf;
}

const char *findAndReplaceNametags(const char *text, const char *name)
{
	static char buf[MAX_SAY_TEXT] = "\0";

	auto str = std::string(text);
	boost::replace_all(str, "[n]", name);

	Q_strncpyz(buf, str.c_str(), sizeof(buf));
	return buf;
}