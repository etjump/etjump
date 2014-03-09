#ifndef commandinterpreter_h__
#define commandinterpreter_h__

#include <map>
#include <string>
#include <boost/function.hpp>
#include "../g_local.hpp"
#include "../g_utilities.hpp"
#include <bitset>

class CommandInterpreter
{
public:
    CommandInterpreter();
    ~CommandInterpreter();

    void PrintFlags(gentity_t *ent);
    void PrintCommandList(gentity_t* ent, std::bitset<256> permissions);
    void PrintHelp(gentity_t *ent, const std::string& command,
        std::bitset<256> permissions);

    struct AdminCommand
    {
        AdminCommand();
        AdminCommand( boost::function<bool(gentity_t *ent, Arguments argv)> handler,
            char flag );

        boost::function<bool(gentity_t *ent, Arguments argv)> handler;
        char flag;
    };

    bool ClientCommand(gentity_t *ent);
private:
    std::map< std::string, AdminCommand > commands_;
};

#endif // commandinterpreter_h__
