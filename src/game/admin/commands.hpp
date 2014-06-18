#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <boost/function.hpp>
#include "..\g_utilities.hpp"
#include <map>

struct gentity_s;
typedef gentity_s gentity_t;

class Commands
{
public:
    Commands();
    bool ClientCommand(gentity_t *ent);
    bool AdminCommand(gentity_t *ent);
private:
    std::map< std::string, boost::function<bool(gentity_t *ent, Arguments argv)> > commands_;
};

#endif 