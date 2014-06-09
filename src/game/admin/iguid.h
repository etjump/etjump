#ifndef iguid_h__
#define iguid_h__

#include <string>
#include "../g_local.hpp"

class IGuid
{
public:
    virtual std::string GetGuid( gentity_t *ent ) = 0;
};

#endif // iguid_h__
