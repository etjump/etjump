#pragma once
#include "etj_irenderable.h"
#include "etj_entity_events_handler.h"
#include "cg_local.h"

namespace ETJump
{
	class ClientCommandsHandler;
	
	class DisplayMaxSpeed : public IRenderable
	{
		float       _maxSpeed{ 0 };
		float       _displayMaxSpeed{ 0 };
		int         _animationStartTime{ 0 };
		vec4_t _color;
		EntityEventsHandler *_entityEventsHandler;

		static void parseColor(const std::string &color, vec4_t &out);
		bool canSkipDraw() const;
	public:
		explicit DisplayMaxSpeed(EntityEventsHandler *entityEventsHandler);
		~DisplayMaxSpeed();
		void render() const override;
		void beforeRender() override;
	};
}
