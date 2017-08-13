#include "etj_banner_system.h"
#include "etj_common.h"
#include "etj_printer.h"

static const char *LocationText[] = {
	"Center",
	"Top",
	"Chat",
	"Left"
};

ETJump::BannerSystem::BannerSystem(Options options): _bannerIdx(0)
{
	_options = options;
	subcribeToRunFrame([=](int levelTime)
	{
		check(levelTime);
	});
	Printer::logPrintLn(
		(boost::format("Initialized banner system\n"
		"- %d banners\n"
		"- %ds interval\n"
		"- %s location"
	) % _options.messages.size() % (_options.interval / 1000) % LocationText[_options.location]).str());
}

void ETJump::BannerSystem::check(int levelTime)
{
	if (_nextBannerTime > levelTime)
	{
		return;
	}

	auto message = _options.messages[_bannerIdx];

	switch (_options.location)
	{
	case Center: 
		Printer::broadcastCenterMessage(message);
		break;
	case Top: 
		Printer::broadcastTopBannerMessage(message);
		break;
	case Chat: 
		Printer::broadcastChatMessage(message);
		break;
	case Left: 
		Printer::broadcastLeftBannerMessage(message);
		break;
	default: 
		Printer::broadcastTopBannerMessage(message);
		break;
	}

	_bannerIdx = (_bannerIdx + 1) % _options.messages.size();
	_nextBannerTime = levelTime + _options.interval;
}

ETJump::BannerSystem::~BannerSystem()
{
}
