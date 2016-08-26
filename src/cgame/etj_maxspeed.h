#pragma once
#include "etj_irenderable.h"
#include "etj_entity_events_handler.h"

namespace ETJump
{
	class ClientCommandsHandler;
	
	class DisplayMaxSpeed : public IRenderable
	{
		float       _maxSpeed{ 0 };
		float       _displayMaxSpeed{ 0 };
		int         _animationStartTime{ 0 };
		EntityEventsHandler *_entityEventsHandler;
	public:
		explicit DisplayMaxSpeed(EntityEventsHandler *entityEventsHandler);
		~DisplayMaxSpeed();
		void render() const override;
		void beforeRender() override;
	};
}
