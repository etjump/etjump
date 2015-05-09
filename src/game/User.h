//
// Created by Jussi on 22.4.2015.
//

#ifndef ETJUMP_USER_H
#define ETJUMP_USER_H

#include <string>
#include <atomic>

/**
 * A single user
 */
class User {
public:
    enum Updated {
        Level = (1 << 0),
        LastSeen = (1 << 1),
        Guid = (1 << 2),
        HardwareId = (1 << 3),
        Name = (1 << 4),
        Ip = (1 << 5),
        Title = (1 << 6),
        Greeting = (1 << 7),
        Commands = (1 << 8)
    };

    static const std::string createTable = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "level INTEGER NOT NULL,"
            "last_seen INTEGER,"
            "guid CHAR(40) UNIQUE NOT NULL,";

    int getLevel() const
    {
        return _level;
    }

    void setLevel(int _level)
    {
        _updated |= Updated::Level;
        User::_level = _level;
    }

    int getLastSeen() const
    {
        return _lastSeen;
    }

    void setLastSeen(int _lastSeen)
    {
        _updated |= Updated::LastSeen;
        User::_lastSeen = _lastSeen;
    }

    std::string &getGuid() const
    {
        return _guid;
    }

    void setGuid(std::string &_guid)
    {
        _updated |= Updated::Guid;
        User::_guid = _guid;
    }

    std::string &getHardwareId() const
    {
        return _hardwareId;
    }

    void setHardwareId(std::string &_hardwareId)
    {
        _updated |= Updated::HardwareId;
        User::_hardwareId = _hardwareId;
    }

    std::string &getName() const
    {
        return _name;
    }

    void setName(std::string &_name)
    {
        _updated |= Updated::Name;
        User::_name = _name;
    }

    std::string &getIp() const
    {
        return _ip;
    }

    void setIp(std::string &_ip)
    {
        _updated |= Updated::Ip;
        User::_ip = _ip;
    }

    std::string &getTitle() const
    {
        return _title;
    }

    void setTitle(std::string &_title)
    {
        _updated |= Updated::Title;
        User::_title = _title;
    }

    std::string &getGreeting() const
    {
        return _greeting;
    }

    void setGreeting(std::string &_greeting)
    {
        _updated |= Updated::Greeting;
        User::_greeting = _greeting;
    }

    std::string &getCommands() const
    {
        return _commands;
    }

    void setCommands(std::string &_commands)
    {
        _updated |= Updated::Commands;
        User::_commands = _commands;
    }

    std::atomic<bool> isActive() const
    {
        return _active;
    }

    void setActive(bool _active)
    {
        User::_active = _active;
    }

    /**
     * Loads the user from the database based on the guid
     * @param The database name
     * @param User's guid
     */
    void Load(std::string database, std::string guid);

    /**
     * Saves the user to database based on the guid
     * @param The database name
     */
    void Save();

private:
    int _level;
    int _lastSeen;
    std::string _guid;
    std::string _hardwareId;
    std::string _name;
    std::string _ip;
    std::string _title;
    std::string _greeting;
    std::string _commands;
    unsigned _updated;

    std::atomic<bool> _active;
};

#endif //ETJUMP_USER_H
