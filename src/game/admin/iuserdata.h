#ifndef iuserdata_h__
#define iuserdata_h__

#include "user.h"

class IUserData
{
public:
    virtual const User *GetUserData( const std::string& guid ) = 0;
    virtual void CreateNewUser( const std::string& guid, const std::string& name, const std::string& hwid ) = 0;
};

#endif // iuserdata_h__
