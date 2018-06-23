#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace ETJump
{
    class ClientCommandsHandler;

    class AwaitedCommandHandler
    {
    public:
        struct AwaitedCommand
        {
            AwaitedCommand(): currentFrameCount(0), requiredFrameCount(0) {}
            int currentFrameCount;
            int requiredFrameCount;
            std::vector<std::string> commands;
        };

        AwaitedCommandHandler(std::shared_ptr<ClientCommandsHandler> consoleCommandsHandler, std::function<void(const char *)> sendConsoleCommand, std::function<void(const char *)> printToConsole);
        ~AwaitedCommandHandler();

        void runFrame();
    private:
        void message(const std::string& message);
        void executeConsoleCommand(const std::string& command);
        void awaitCommand(const std::vector<std::string>& args);
        std::shared_ptr<ClientCommandsHandler> _consoleCommandsHandler;
        std::vector<std::unique_ptr<AwaitedCommand>> _awaitedCommands;
        std::function<void(const char*)> _sendConsoleCommand;
        std::function<void(const char*)> _printToConsole;
    };
}
