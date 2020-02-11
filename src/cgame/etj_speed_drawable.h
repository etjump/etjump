#pragma once
#include "etj_irenderable.h"
#include "cg_local.h"
#include <string>
#include <list>

namespace ETJump
{
	class DisplaySpeed : public IRenderable
	{
		struct StoredSpeed
		{
			int time;
			float speed;
		};

		std::list<StoredSpeed> _storedSpeeds;
		float _maxSpeed{ 0 };
		vec4_t _color;
		bool _shouldDrawShadow{ false };
		static void parseColor(const std::string &color, vec4_t &out);
		void resetMaxSpeed();
		void checkShadow();
		void startListeners();
		std::string getStatus() const;
		bool canSkipDraw() const;
		void popOldStoredSpeeds();
		float calcAvgAccel() const;
	public:
		DisplaySpeed();
		~DisplaySpeed();
		void render() const override;
		void beforeRender() override;
	};
}
