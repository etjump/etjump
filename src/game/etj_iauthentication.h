#ifndef IAUTHENTICATION_HH
#define IAUTHENTICATION_HH
#include <string>
#include "etj_user.h"

/**
 * Session interface for database
 */

class IAuthentication
{
public:
	virtual ~IAuthentication()
	{
	};
	virtual const std::string GetMessage() const                                                          = 0;
	virtual bool AddUser(const std::string& guid, const std::string& hardwareId, const std::string& name) = 0;
	virtual bool AddNewHardwareId(int id, const std::string& hardwareId)                                  = 0;
	virtual bool BanUser(std::string const& name, std::string const& guid, std::string const& hardwareId, std::string const& ip,
	                     std::string const& bannedBy, std::string const& banDate, unsigned expires, std::string const& reason) = 0;
	virtual bool IsBanned(const std::string& guid, const std::string& hardwareId)                                              = 0;
	virtual bool IsIpBanned(const std::string& ip)                                                                             = 0;
	virtual bool UserExists(unsigned id)                                                                                       = 0;
	virtual bool UserExists(const std::string& guid)                                                                           = 0;
	virtual const User_s *GetUserData(const std::string& guid) const                                                           = 0;
	virtual const User_s *GetUserData(int id) const                                                                            = 0;
	virtual bool SetLevel(int id, int level)                                                                                   = 0;
	virtual void NewName(int id, const std::string& name)                                                                      = 0;
	virtual bool UpdateLastSeen(int id, int lastSeen)                                                                          = 0;
	virtual int ResetUsersWithLevel(int level)                                                                                 = 0;
};

#endif
