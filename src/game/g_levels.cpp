#include "g_levels.h"
#include "g_utilities.h"
#include <string>
#include <fstream>
#include <vector>
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
    
    BeginBufferPrint();
    for( vector<Level*>::size_type i = 0; i < levels_.size(); i++ ) {
        BufferPrint(NULL, levels_.at(i)->name + NEWLINE);
    }
    FinishBufferPrint(NULL, false);
    G_LogPrintf(LINE);
    return true;
}

void LevelDatabase::Reset()
{
    // Shared ptr will delete objects
    levels_.clear();
}

bool LevelDatabase::Update( int level, const std::string& cmds,const std::string& name, const std::string& greeting, int updatedValues )
{
    enum UPDATED_VALUE {
        CMDS,
        NAME,
        GREETING
    };


    LevelIterator it = levels_.begin();

    while(it != levels_.end()) {
        if(it->get()->level == level) {
            break;
        }
        it++;
    }

    if( it == levels_.end() ) {
        return false;
    }


    if(updatedValues & (1 << CMDS)) {
        (*it).get()->commands = cmds;
    } 
    
    if(updatedValues & (1 << NAME)) {
        (*it).get()->name = name;
    } 
    
    if(updatedValues & (1 << GREETING)) {
        (*it).get()->greeting = greeting;
    }
    WriteConfig();
    return true;
}

bool LevelDatabase::WriteConfig()
{
    vector<shared_ptr<Level> >::const_iterator it = levels_.begin();
    std::ofstream config(LEVELS_PATH.c_str());

    if(!config) {
        return false;
    }

    while( it != levels_.end() ) {
        config << "[level]\n";
        config << "level = " << (*it)->level << NEWLINE;
        config << "name = " << (*it)->name << NEWLINE;
        config << "commands = " << (*it)->commands << NEWLINE;
        config << "greeting = " << (*it)->greeting << NEWLINE << NEWLINE;
        it++;
    }

    config.close();
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
