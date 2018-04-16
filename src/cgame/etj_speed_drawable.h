#pragma once
#include "etj_irenderable.h"
#include "cg_local.h"
#include <string>

namespace ETJump
{
	class DisplaySpeed : public IRenderable
	{
		float _maxSpeed{ 0 };
		vec4_t _color;
		bool _shouldDrawShadow{ false };
		static void parseColor(const std::string &color, vec4_t &out);
		void resetMaxSpeed();
		void checkShadow();
		void startListeners();
		std::string getStatus() const;
	public:
		DisplaySpeed();
		~DisplaySpeed();
		void render() const override;
		void beforeRender() override;
	};
}
