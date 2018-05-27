#pragma once
#include <array>
#include <map>
#include <string>
#include <vector>
#include "etj_irenderable.h"
#include "../game/q_shared.h"

typedef struct playerState_s playerState_t;

namespace ETJump
{
	class ClientCommandsHandler;

	class OverbounceWatcher : public IRenderable
	{
	public:
		using Coordinate = std::array<float, 3>;

		explicit OverbounceWatcher(ClientCommandsHandler *clientCommandsHandler);
		~OverbounceWatcher();

		void render() const override;
		void beforeRender() override;

		vec4_t _color;

	private:
		ClientCommandsHandler *_clientCommandsHandler;

		std::map<std::string, Coordinate> _positions;

		// Currently displayed position
		Coordinate *_current;

		// saves the position with name
		void save(const std::string& name, Coordinate coordinate);

		// stop displaying anything
		void reset();

		// loads the position to currently displayed position
		// if position is not found, returns false
		bool load(const std::string& name);

		// lists all available positions
		std::vector<std::string> list() const;

		static const playerState_t *getPlayerState();

		// checks if it's possible to overbounce with current velocity and height 
		static bool isOverbounce(float vel, float currentHeight,
			float finalHeight, float rintv,
			float psec, int gravity);

		// checks if current surface has disabled overbounce
		static bool surfaceAllowsOverbounce(trace_t* trace);
	};
}



