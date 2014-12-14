/*
 * Prints a message of the day when client connects to server.
 * g_motdFile is the file where the motd is defined. The format is 
 * in json.
 * Two messages are sent: the actual motd and also a message in chat 
 * to inform that there's a motd in the console.
 * Example format:
 * {
 *   "chat_message": "Welcome to the server! Check out console for more information.",
 *   "console_message": "Hi!"
 * }
 */

#include <string>
#include <ostream>

class Motd
{
public:
    Motd();
    ~Motd();
    void Initialize();
    void GenerateMotdFile();

private:
    bool initialized_;
    std::string chatMessage_;
    std::string motd_;
};