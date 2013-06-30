#ifndef g_admin_h__
#define g_admin_h__

#include <string>
#include "g_local.hpp"
#include "g_utilities.hpp"

struct AdminCommand_s
{
    std::string keyword;
    bool (*handler)(gentity_t *ent, Arguments argv);
    char flag;
    std::string function;
    std::string syntax;
};

#endif // g_admin_h__
