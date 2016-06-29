#include "etj_user.h"

ETJump::User::User(int64_t id, const std::string& guid): _id(id), _guid(guid), _level(0), _lastSeen(0)
{
}

ETJump::User::User(): _id(0), _guid("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"), _level(0), _lastSeen(0)
{
}

ETJump::User::~User()
{
}
