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
			RunFrame
		};

		struct ServerEventHandler
		{
			EventHandle handle;
			ServerEventType type;
		};

		enum class ClientEventType
		{
			ClientBegin,
			ClientThink,
			ClientConnect,
			ClientUserinfoChanged,
			ClientDisconnect,
			NumClientEvents
		};

		EventAggregator();
		~EventAggregator();

		// Subscribes to a server event. 
		// @return handle to unsubscribe from the event
		int subscribe(ServerEventType eventType, std::function<void()> callback);
		// Subscribes to a client event. 
		// @return handle to unsubscribe from the event
		int subscribe(ClientEventType eventType, std::function<void(int clientNum)> callback);
		// unsubscribes from the event.
		void unsubcribe(int eventHandle);
		// called when the event occurs
		void clientEvent(ClientEventType eventType, int clientNum);
		void serverEvent(ServerEventType eventType);
	private:
		struct ServerCallback
		{
			bool inUse;
			EventHandle handle;
			ServerEventType type;
			std::function<void()> callback;
		};

		struct ClientCallback
		{
			bool inUse;
			EventHandle handle;
			ClientEventType type;
			std::function<void(int clientNum)> callback;
		};

		EventHandle _nextFreeHandle;
		std::vector<ServerCallback> _serverCallbacks;
		std::vector<ClientCallback> _clientCallbacks;
	};
}



