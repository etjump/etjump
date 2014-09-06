#ifndef LEVELS_HPP
#define LEVELS_HPP

struct gentity_s;
typedef gentity_s gentity_t;

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

class Levels
{
public:

    struct Level
    {
        Level(int level, const std::string& name, const std::string& greeting,
        const std::string& commands);

        int level;
        std::string name;
        std::string commands;
        std::string greeting;
    };

    Levels();
    ~Levels();

    bool Add(int level, const std::string name, const std::string commands, const std::string greeting);
    bool Edit(int level, std::string const& name, std::string const& commands, std::string const& greeting, int updated);
    bool Delete(int level);
    bool ReadFromConfig();
    static bool SortByLevel(boost::shared_ptr<Level> const lhs, boost::shared_ptr<Level> const rhs);
    bool WriteToConfig();
    std::string ErrorMessage();
    void PrintLevels();
    const Level *GetLevel(int level);
    bool LevelExists(int level) const;
    void PrintLevelInfo(gentity_t *ent);
    void PrintLevelInfo(gentity_t *ent, int level);
private:
    typedef std::vector< boost::shared_ptr< Level > >::const_iterator ConstIter;
    typedef std::vector< boost::shared_ptr< Level > >::iterator Iter;
    bool CreateDefaultLevels();
    ConstIter FindConst(int level);
    Iter Find(int level);
    std::vector< boost::shared_ptr< Level > > levels_;
    std::string errorMessage;
    boost::shared_ptr<Level> dummyLevel_;
};

#endif