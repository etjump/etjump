//
// Created by Jussi on 22.4.2015.
//

#include "Users.h"

Users::Users()
{
    for (auto& user : _users) {
        user.setActive(false);
    }
}