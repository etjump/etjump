#ifndef g_levels_h__
#define g_levels_h__

#include "g_local.hpp"

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

struct Level {
    bool operator<(const int& rhs);
    int level;
    std::string name;
    std::string commands;
    std::string greeting;
};

typedef std::vector<boost::shared_ptr<Level> >::iterator LevelIterator;
typedef std::vector<boost::shared_ptr<Level> >::const_iterator ConstLevelIterator;

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
    bool Update(int level, const std::string& cmds,const std::string& name, const std::string& greeting, int updatedValues);
private:
    std::vector<boost::shared_ptr<Level> > levels_;
};

#endif // g_levels_h__
