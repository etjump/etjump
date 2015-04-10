//
// Created by Jussi on 7.4.2015.
//

#ifndef ETJUMP_UTILITIES_H
#define ETJUMP_UTILITIES_H

#include <vector>

class Utilities {
public:
    /**
     * Returns the list of spectators spectating client
     * @clientNum The client
     */
    static std::vector<int> getSpectators(int clientNum);

    /**
     * Disables all cheats for player. Resets saves
     * @clientNum the client
     * TODO: Should create a Game-object that handles these
     */
    static void startRun(int clientNum);

    /**
     * Enables all cheats for player.
     * @clientNum the player
     */
    static void stopRun(int clientNum);

private:
};


#endif //ETJUMP_UTILITIES_H
