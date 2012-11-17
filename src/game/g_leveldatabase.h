#ifndef LEVELDATABASE_H
#define LEVELDATABASE_H

#include <map>
#include <string>
using std::string;
using std::map;

class LevelDatabase {
public:

    LevelDatabase();
    ~LevelDatabase();

    struct admin_level_t {
        admin_level_t(string n, string c, string g);
        string name;
        string commands;
        string greeting;
    };

    enum types {
        NAME,
        COMMANDS,
        GREETING
    };

    bool addLevel(int level, string name, string commands, string greeting);
    bool updateLevel(int level, string str, int type);
    bool deleteLevel(int level);

    bool readInteger(std::stringstream& current_line, int& integer);
    bool readString(std::stringstream& current_line, string& str);
    bool readConfig();

    string getAll(int level);

    int levelCount() const;

private:

    void clearDatabase();
    void writeDefaultConfig();
    void createLevelZero();

    map<int, admin_level_t*> levels_;    

};

#endif