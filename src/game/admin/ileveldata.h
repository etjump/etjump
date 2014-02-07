#ifndef ileveldata_h__
#define ileveldata_h__




#include <string>

class ILevelData
{
public:
    struct LevelInformation
    {
        LevelInformation(): isValid(false)
        {

        }
        bool isValid;
        std::string name;
        std::string commands;
        std::string greeting;
    };
    virtual const LevelInformation *GetLevelInformation( int level ) = 0;
};

#endif // ileveldata_h__