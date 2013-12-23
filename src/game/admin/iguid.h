#ifndef iguid_h__
#define iguid_h__

#include <string>

struct gentity_s;
typedef gentity_s gentity_t;

class IGuid
{
public:
    virtual std::string GetGuid( gentity_t *ent ) = 0;
};

#endif // iguid_h__
