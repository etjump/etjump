#ifndef g_leveldb_h__
#define g_leveldb_h__

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

struct LevelData_s
{
    int level;
    std::string name;
    std::string commands;
    std::string greeting;
};

typedef boost::shared_ptr<LevelData_s> LevelData;

class LevelDatabase
{
public:
    LevelDatabase();
    ~LevelDatabase();
    void ReadConfig();
private:
    std::vector<LevelData> levels_;
};

#endif // g_leveldb_h__
