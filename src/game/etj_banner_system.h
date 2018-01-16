#pragma once
#include <vector>

namespace ETJump
{
	class BannerSystem
	{
	public:
		enum Location
		{
			Center,
			Top,
			Chat,
			Left
		};

		struct Options
		{
			Options(): interval(60), location(Top) {}
			int interval;
			Location location;
			std::vector<const char*> messages;
		};

		BannerSystem(Options options);
		~BannerSystem();
		void check(int levelTime);
	private:
		Options _options;
		int _bannerIdx;
		int _nextBannerTime;
	};
}



