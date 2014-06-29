#ifndef GAME_HPP
#define GAME_HPP

#include <boost/shared_ptr.hpp>
class Levels;
class Session;
class Commands;
class SaveSystem;

struct Game
{
    Game();

    boost::shared_ptr<Levels> levels;
    boost::shared_ptr<Session> session;
    boost::shared_ptr<Commands> commands;
    boost::shared_ptr<SaveSystem> saves;
};

#endif 
