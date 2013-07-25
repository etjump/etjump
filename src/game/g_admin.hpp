#ifndef g_admin_h__
#define g_admin_h__

#include <string>
#include <boost/shared_ptr.hpp>

namespace Admin
{
    struct UserData_s
    {
        UserData_s();
        int id;
        int level;
        std::string hwid;
        std::string name;
        std::string personalCmds;
        std::string personalTitle;
        std::string personalGreeting;
    };

    typedef boost::shared_ptr<UserData_s> UserData;
}

#endif // g_admin_h__
