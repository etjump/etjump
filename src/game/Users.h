//
// Created by Jussi on 22.4.2015.
//

#ifndef ETJUMP_USERS_H
#define ETJUMP_USERS_H

#include <string>
#include <array>
#include "User.h"

/**
 * A database of all the users
 */
class Users {
public:
    /**
     * Initializes the users database. Sets the status
     * of all players to inactive
     */
    Users();
    /*
     * When the user is connected we don't know if it's a new user
     * or an existing one, so we just set the user status to unknown
     * @param clientNum
     */
    void connect(int clientNum);

    /**
     * When the user has connected properly, we need to check whether the user already
     * exists and do appropriate tasks.
     * @param clientNum User's client number
     * @param guid User's globally unique identifier
     * @param hardwareId User's hardware identifier
     */
    void connected(int clientNum, const std::string &guid, const std::string &hardwareId);

    /**
     * When user disconnects, we need to set the status to disconnected
     * and do other appropriate tasks.
     * @param clientNum User's client number
     */
    void disconnected(int clientNum);

    /**
     * Returns a const pointer to the user specified by clientNum
     * @param clientNum The user's clientNum
     * @return User* or nullptr if user is inactive
     */
    const User* getUser(int clientNum) const;

private:
    std::array<User, 64> _users;
};


#endif //ETJUMP_USERS_H
