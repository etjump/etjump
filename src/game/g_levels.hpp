#ifndef g_levels_h__
#define g_levels_h__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

struct LevelData_s
{
    LevelData_s(int level, const std::string& name,
        const std::string& greeting, const std::string& commands);
    LevelData_s();
    int level;
    std::string name;
    std::string greeting;
    std::string commands;
};

typedef boost::shared_ptr<LevelData_s> LevelData;
typedef std::vector<LevelData>::iterator LevelIter;
typedef std::vector<LevelData>::const_iterator ConstLevelIter;

class LevelDatabase
{
public:
    LevelDatabase();
    ~LevelDatabase();

    void ReadLevels();
    void WriteLevels();

    void AddLevel(int level);
    void AddLevel(int level, const std::string& commands,
        const std::string& greeting, const std::string& title);

    std::string Permissions(int level);
    std::string Greeting(int level);
    std::string Name(int level);

    bool LevelExists(int level);
private:
    void CreateDefaultLevels();
    std::vector<LevelData> levels_;
};

#endif // g_levels_h__
