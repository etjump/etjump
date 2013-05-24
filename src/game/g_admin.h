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

const enum AdminFlags {
	SILENT_COMMANDS = '/',
};

bool IsAllowed(gentity_t *caller, gentity_t *target, bool equalIsHigher = true);