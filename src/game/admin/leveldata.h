#ifndef leveldata_h__
#define leveldata_h__

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "ileveldata.h"

struct gentity_s;
typedef struct gentity_s gentity_t;

class LevelData : public ILevelData
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

    bool AddLevel( int level, const std::string& name, 
        const std::string& commands,
        const std::string& greeting  );

    bool EditLevel( int level, const std::string& newName,
        const std::string& newCommands, 
        const std::string& newGreeting,
        int updated );

    bool DeleteLevel( int level );

    void PrintLevels( gentity_t *ent );

    const ILevelData::LevelInformation *GetLevelInformation( int level );

    void ReadLevels();
    void WriteLevels();
private:
    void CreateDefaultLevels();
    static bool SortByLevel(const boost::shared_ptr<Level>& lhs, const boost::shared_ptr<Level>& rhs);
    std::vector< boost::shared_ptr< Level > > levels_;
};

#endif // leveldata_h__
