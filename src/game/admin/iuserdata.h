#ifndef iuserdata_h__
#define iuserdata_h__

#include "user.h"
#include <vector>

class IUserData
{
public:
    virtual const User *GetUserData( const std::string& guid ) = 0;
    virtual int CreateNewUser( const std::string& guid, const std::string& name, const std::string& hwid ) = 0;
    virtual void UpdateLastSeen(int id, int seen) = 0;
    virtual void UpdateLevel(const std::string& guid, int level) = 0;
    virtual bool UserListData(int page, std::vector<std::string>& linesToPrint) = 0;
    virtual void UserIsOnline(const std::string& guid) = 0;
    virtual bool UpdateUser(const std::string& guid, const std::string& commands, const std::string& greeting, const std::string& title, int updated) = 0;
};

#endif // iuserdata_h__
