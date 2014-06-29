#include "game.hpp"
#include "session.hpp"
#include "commands.hpp"
#include "../g_save.hpp"
#include "levels.hpp"

Game::Game()
{
    levels = boost::shared_ptr<Levels>(new Levels());
    session = boost::shared_ptr<Session>(new Session());
    commands = boost::shared_ptr<Commands>(new Commands());
    saves = boost::shared_ptr<SaveSystem>(new SaveSystem(session.get()));
}