#ifndef g_levels_h__
#define g_levels_h__

#include "g_local.hpp"

#include <string>
#include <vector>
#include <bitset>
#include <boost/shared_ptr.hpp>

struct Level {
    bool operator<(const int& rhs);
    bool Level::operator<( const Level& rhs );
    int level;
    std::string name;
    std::string commands;
    std::string greeting;
};

typedef std::vector<boost::shared_ptr<Level> >::iterator LevelIterator;
typedef std::vector<boost::shared_ptr<Level> >::const_iterator ConstLevelIterator;
const size_t MAX_CMDS = 256;

void ResetLevel(Level& lvl);
class LevelDatabase
{
public:
    LevelDatabase();
    ~LevelDatabase();
    Level Get(int level);
    void Reset();
    bool ReadConfig();
    bool WriteConfig();
    bool AddLevel(int level);
    bool UpdateLevel(int level, 
        const std::string& cmds,
        const std::string& name, 
        const std::string& greeting, 
        int updatedAttributes,
        int editMode);
    void PrintLevelToConsole(int level, gentity_t *ent) const;
    std::string Error() const;
    std::bitset<MAX_CMDS> Permissions(int level) const;
    std::string Name(int level) const;
private:
    ConstLevelIterator ConstFindLevelIter(int level) const;
    LevelIterator FindLevelIter(int level);
    std::vector<boost::shared_ptr<Level> > levels_;
    std::string error_;
};

#endif // g_levels_h__
