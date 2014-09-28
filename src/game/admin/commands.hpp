#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <boost/function.hpp>
#include "../g_utilities.hpp"
#include <map>

class Commands
{
public:
    Commands();
    bool ClientCommand(gentity_t *ent, std::string command);
    bool AdminCommand(gentity_t *ent);
    bool List(gentity_t *ent);
private:
    std::map< std::string, boost::function<bool(gentity_t *ent, Arguments argv)> > commands_;
    std::map< std::string, std::pair<boost::function<bool(gentity_t *ent, Arguments argv)>, char> > adminCommands_;
};

#endif 


