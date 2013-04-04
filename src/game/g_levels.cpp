#include "g_levels.h"
#include "g_utilities.h"
#include <string>
#include <fstream>
#include <vector>
#include <bitset>
#include <boost/algorithm/string.hpp>
using std::string;
using std::vector;
using boost::shared_ptr;

LevelDatabase::LevelDatabase()
{

}

LevelDatabase::~LevelDatabase()
{

}

bool LevelComparator(const shared_ptr<Level>& lhs, const shared_ptr<Level>& rhs) 
{
    return lhs->level < rhs->level;
}

const string LEVELS_PATH = "etjump/levels.cfg";
const char LINE[] = 
    "------------------------------------------------------------\n";


bool LevelDatabase::ReadConfig()
{
    if( !*g_admin.string ) {
        return true;
    }

    unsigned line = 1;
    bool levelOpen = false;
    bool success = true;
    Level temporaryLevel;
    std::ifstream config(LEVELS_PATH.c_str());

    if(!config) {
        G_LogPrintf("LevelDB: failed to open config file.\n");
        return false;
    }

    Reset();

    G_LogPrintf(LINE);
    G_LogPrintf("ETJump: loading level database\n");

    while(config.peek() != EOF) {
        string token;

        config >> token;
        if(token == "[level]") {
            if(levelOpen) {
                try {
                    shared_ptr<Level> newLevel(new Level(temporaryLevel));
                    levels_.push_back(newLevel);
                } catch (...) {
                    G_LogPrintf("LevelDB: failed to allocate memory for level\n");
                    success = false;
                    break;
                }
                ResetLevel(temporaryLevel);
            }
            levelOpen = true;
        } else if( token == "level" ) {
            config >> token;
            if( token != "=" ) {
                G_LogPrintf("LevelDB: error on line %d: expected =\n", line);
                success = false;
                break;
            }
            config >> token;
            if(!StringToInt(token, temporaryLevel.level)) {
                G_LogPrintf("LevelDB: error on line %d: expected an integer\n", line);
                success = false;
                break;
            }
        } else if( token == "name" ) {
            config >> token;
            if( token != "=" ) {
                G_LogPrintf("LevelDB: error on line %d: expected =\n", line);
                success = false;
                break;
            }
            if( config.peek() == EOF ) {
                temporaryLevel.name = "";
                break;
            }
            std::getline(config, token);
            boost::trim(token);
            temporaryLevel.name = token;
        } else if( token == "commands" ) {
            config >> token;
            if( token != "=" ) {
                G_LogPrintf("LevelDB: error on line %d: expected =\n", line);
                success = false;
                break;
            }
            if( config.peek() == EOF ) {
                temporaryLevel.commands = "";
                break;
            }
            std::getline(config, token);
            boost::trim(token);
            temporaryLevel.commands = token;
        } else if( token == "greeting" ) {
            config >> token;
            if( token != "=" ) {
                G_LogPrintf("LevelDB: error on line %d: expected =\n", line);
                success = false;
                break;
            }
            if( config.peek() == EOF ) {
                temporaryLevel.greeting = "";
                break;
            }
            std::getline(config, token);
            boost::trim(token);
            temporaryLevel.greeting = token;
        }
        line++;
    }

    if(levelOpen) {
        try {
            shared_ptr<Level> newLevel(new Level(temporaryLevel));
            levels_.push_back(newLevel);
        } catch (...) {
            G_LogPrintf("LevelDB: failed to allocate memory for level\n");
            success = false;
        }
    }

    config.close();

    if(!success) {
        // Shared ptr handles memory management
        levels_.clear();
        G_LogPrintf(LINE);
        return false;
    }

    G_LogPrintf("ETJump: loaded %d levels.\n", levels_.size());
    
    G_LogPrintf(LINE);

    // Let's sort the DB aswell
    std::sort(levels_.begin(), levels_.end(), LevelComparator);

    return true;
}

void LevelDatabase::Reset()
{
    // Shared ptr will delete objects
    levels_.clear();
}

bool LevelDatabase::UpdateLevel( int level, const std::string& cmds, 
                                const std::string& name, 
                                const std::string& greeting, 
                                int updatedAttributes, int editMode )
{
    LevelIterator it = FindLevelIter(level);

    // TODO: maybe create a new level?
    if( it == levels_.end() ) {
        error_ = "couldn't find level.";
        return false;
    }

    if(updatedAttributes & (1 << COMMANDS)) {
        if( editMode == CLEAR ) {
            (*it).get()->commands.clear();
        } else if( editMode == APPEND ) {
            (*it).get()->commands += cmds;
        } else if( editMode == REPLACE ) {
            (*it).get()->commands = cmds;
        }
        std::sort(it->get()->commands.begin(), it->get()->commands.end());
        RemoveDuplicates(it->get()->commands);
    }

    if(updatedAttributes & (1 << GREETING)) {
        if( editMode == CLEAR ) {
            (*it).get()->greeting.clear();
        } else {
            (*it).get()->greeting = greeting;
        }
    }

    if (updatedAttributes & (1 << NAME)) {
        if( editMode == CLEAR ) {
            (*it).get()->name.clear();
        } else {
            (*it).get()->name = name;
        }
    }
    WriteConfig();
    return true;
}

bool LevelDatabase::WriteConfig()
{
    std::ofstream config(LEVELS_PATH.c_str());

    if(!config) {
        return false;
    }

    std::sort(levels_.begin(), levels_.end(), LevelComparator);

    vector<shared_ptr<Level> >::const_iterator it = levels_.begin();
    while( it != levels_.end() ) {
        config << "[level]\n";
        config << "level = " << (*it)->level << NEWLINE;
        config << "name = " << (*it)->name << NEWLINE;
        config << "commands = " << (*it)->commands << NEWLINE;
        config << "greeting = " << (*it)->greeting << NEWLINE << NEWLINE;
        it++;
    }

    config.close();
    return true;
}

std::string LevelDatabase::Error() const
{
    return error_;
}

void LevelDatabase::PrintLevelToConsole( int level, gentity_t *ent ) const
{
    ConstLevelIterator it = ConstFindLevelIter(level);

    if(it == levels_.end()) {
        ConsolePrintTo(ent, "LevelDB: level not found.");
        return;
    }

    BeginBufferPrint();
    BufferPrint(ent, "^7[level]\n");
    BufferPrint(ent, "^7level    = " + IntToString(it->get()->level));
    BufferPrint(ent, "\n^7name     = " + it->get()->name);
    BufferPrint(ent, "\n^7commands = " + it->get()->commands);
    BufferPrint(ent, "\n^7greeting = " + it->get()->greeting);
    FinishBufferPrint(ent, true);
}

bool LevelDatabase::AddLevel( int level )
{
    if( ConstFindLevelIter(level) != levels_.end() ) {
        error_ = "level exists.";
        return false;
    }

    shared_ptr<Level> newLevel(new Level);
    newLevel->level = level;
    levels_.push_back(newLevel);
    WriteConfig();
    return true;
}

ConstLevelIterator LevelDatabase::ConstFindLevelIter( int level ) const
{
    ConstLevelIterator it = levels_.begin();
    while(it != levels_.end()) {

        if(it->get()->level == level) {
            break;
        }

        it++;
    }
    return it;
}

LevelIterator LevelDatabase::FindLevelIter( int level )
{
    LevelIterator it = levels_.begin();
    while(it != levels_.end()) {

        if(it->get()->level == level) {
            break;
        }

        it++;
    }
    return it;
}

std::bitset<MAX_CMDS> LevelDatabase::Permissions( int level ) const
{
    std::bitset<MAX_CMDS> permissions;
    permissions.reset();

    ConstLevelIterator it = levels_.begin();

    // Levels are always sorted
    while(it != levels_.end() && it->get()->level <= level) {
        const char* flags = it->get()->commands.c_str();

        while(*flags) {

            // Shouldn't happen but just to be sure
            if(*flags < 0) {
                *flags++;
                continue;
            }

            if(*flags == '-') {
                while(*flags++) {
                    if(*flags == '+') {
                        break;
                    }
                    permissions.set(*flags, true);
                }
            } else if(*flags == '*') {
                // Clear allowed
                permissions.reset();
                // Make everything allowed
                permissions.flip();
                while(*flags++) {
                    // Disable ones listed after *
                    permissions.set(*flags, false);
                }
            }

            permissions.set(*flags, true);

            flags++;
        }
        it++;
    }

    return permissions;
}

std::string LevelDatabase::Name( int level ) const
{
    ConstLevelIterator it = ConstFindLevelIter(level);
    
    if(it != levels_.end()) {
        if(it->get()->name.length() != 0) {
            ChatPrintTo(NULL, it->get()->name);
            return it->get()->name;
        }
    }

    return "^7undefined";
}

void ResetLevel( Level& lvl )
{
    lvl.level = -1;
    lvl.name.clear();
    lvl.commands.clear();
    lvl.greeting.clear();
}

bool Level::operator<( const int& rhs )
{
    return this->level < rhs;
}

bool Level::operator<( const Level& rhs ) 
{
    return this->level < rhs.level;
}
