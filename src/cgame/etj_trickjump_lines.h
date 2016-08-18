#pragma once

#include <map>

namespace ETJump
{
	class ClientCommandsHandler;

	class TrickjumpLines
	{
	public:
		typedef float vec_t;
		typedef unsigned char vec_c;
		typedef vec_t vec3_t[3];
		typedef vec_t vec4_t[4];
		typedef vec_c vec4_c[4];

		// A single node in a tjline
		struct Node
		{
			vec3_t coordinate;
			float speed;
		};

		struct Route
		{
			std::string name;
			// A trail starts when the player jumps and ends when the player
			// lands on ground
			std::vector<std::vector<Node>> trails;
			vec4_c color;
			float width;
		};

		TrickjumpLines(ClientCommandsHandler *clientCommandsHandler, ClientCommandsHandler *serverCommandsHandler);
		~TrickjumpLines();

	private:
		void subscribeToClientCommands();
		void unsubscribeFromClientCommands();

		void subscribeToServerCommands();
		void unsubscribeFromServerCommands();

		// Lists all currently available tjline file (files with .json extension
		// in etjump/tjlines/
		void listFiles();

		ClientCommandsHandler *_clientCommandsHandler;
		ClientCommandsHandler *_serverCommandsHandler;

		// namespaced routes
		std::map<std::string, Route> _routes;
	};
}

