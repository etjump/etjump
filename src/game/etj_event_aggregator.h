#pragma once
#include <functional>
#include <vector>
#include <map>

namespace ETJump
{
	class EventAggregator
	{
	public:
		typedef int EventHandle;

		enum class ServerEventType
		{
			InitGame,
			ShutdownGame,
			RunFrame,
			NumServerEvents
		};

		struct ServerEventHandler
		{
			EventHandle handle;
			ServerEventType 
		};

		enum class ClientEventType
		{
			ClientBegin,
			ClientThink,
			ClientConnect,
			ClientDisconnect,
			NumClientEvents
		};

		EventAggregator();
		~EventAggregator();

		// Subscribes to a server event. 
		// @return handle to unsubscribe from the event
		int subscribe(ServerEventType, std::function<void()>);
		// Subscribes to a client event. 
		// @return handle to unsubscribe from the event
		int subscribe(ClientEventType, std::function<void(int clientNum)>);
		// unsubscribes from the event.
		void unsubcribe(int eventHandle);
	private:
		int _nextFreeHandle;
		std::vector < std::pair<int, std::function<void()> > > _serverCallbacks;
		std::vector < std::pair<int, std::function<void(int clientNum)> > > _clientCallbacks;
	};
}



