#include <string>
#include "g_utilities.h"
#include "g_local.hpp"

struct Command {
    std::string keyword;
    bool (* handler)(gentity_t *ent, Arguments argv);
    char flag;
    std::string function;
    std::string syntax;
};