#pragma once
#include <functional>
#include <vector>

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
			// called with int clientNum
			ClientBegin,
			// called with int clientNum
			ClientThink,
			// called with int clientNum, qboolean firstTime, qboolean isBot
			ClientConnect,
			// called with int clientNum
			ClientUserinfoChanged,
			// called with int clientNum
			ClientDisconnect
		};

		class Payload
		{
		public:
		};

		class ClientNumPayload : public Payload
		{
		public:
			int clientNum;
		};

		class ClientConnectPayload : public Payload
		{
		public: 
			int clientNum;
			int firstTime;
			int isBot;
		};

		class RunFramePayload : public Payload
		{
		public:
			int levelTime;
		};

		EventAggregator();
		~EventAggregator();

		// Subscribes to a server event. 
		// @return handle to unsubscribe from the event
		int subscribe(ServerEventType eventType, std::function<void(const Payload *)> callback);
		// Subscribes to a client event. 
		// @return handle to unsubscribe from the event
		int subscribe(ClientEventType eventType, std::function<void(const Payload *)> callback);
		// unsubscribes from the event.
		void unsubcribe(int eventHandle);
		// called when the event occurs
		// Note: payload will not be owned by the event aggregator => it must not be
		// deleted by the caller before function call returns.
		void clientEvent(ClientEventType eventType, const Payload *payload);
		void serverEvent(ServerEventType eventType, const Payload *payload);
	private:
		struct ServerCallback
		{
			bool inUse;
			EventHandle handle;
			ServerEventType type;
			std::function<void(const Payload *)> callback;
		};

		struct ClientCallback
		{
			bool inUse;
			EventHandle handle;
			ClientEventType type;
			std::function<void(const Payload *)> callback;
		};

		EventHandle _nextFreeHandle;
		std::vector<ServerCallback> _serverCallbacks;
		std::vector<ClientCallback> _clientCallbacks;
	};
}


