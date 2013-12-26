#ifndef leveldata_h__
#define leveldata_h__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

class LevelData
{
public:
    LevelData();
    ~LevelData();
    struct Level
    {
        Level(int level, const std::string& name, 
            const std::string& greeting, const std::string& commands);

        int level;
        std::string name;
        std::string greeting;
        std::string commands;
    };

    void AddLevel( int level, const std::string& name, 
        const std::string& commands,
        const std::string& greeting  );

    void ReadLevels();
    void WriteLevels();
private:
    void CreateDefaultLevels();
    std::vector< boost::shared_ptr< Level > > levels_;
};

#endif // leveldata_h__
