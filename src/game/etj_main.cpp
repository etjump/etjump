extern "C" {
#include "g_local.h"
}

#include <memory>
#include "etj_banner_system.hpp"
#include "printer.hpp"

namespace 
{
	// each of these will be called on every frame
	std::vector<std::function<void(int levelTime)>> runFrameCallbacks;

	std::unique_ptr<ETJump::BannerSystem> bannerSystem = nullptr;
	void InitBannerSystem()
	{
		ETJump::BannerSystem::Options options;
		options.interval = g_bannerTime.integer;
		options.location = static_cast<ETJump::BannerSystem::Location>(g_bannerLocation.integer);
		if (strlen(g_banner1.string) > 0) options.messages.push_back(g_banner1.string); 
		if (strlen(g_banner2.string) > 0) options.messages.push_back(g_banner2.string);
		if (strlen(g_banner3.string) > 0) options.messages.push_back(g_banner3.string);
		if (strlen(g_banner4.string) > 0) options.messages.push_back(g_banner4.string); 
		if (strlen(g_banner5.string) > 0) options.messages.push_back(g_banner5.string); 

		bannerSystem = std::unique_ptr<ETJump::BannerSystem>(new ETJump::BannerSystem(options));
	}

	void ShutdownBannerSystem()
	{
		Printer::LogPrintln("Banner system shut down");
		bannerSystem = nullptr;
	}
}

namespace ETJump
{
	int subcribeToRunFrame(std::function<void(int)> callback)
	{
		runFrameCallbacks.push_back(callback);
		return runFrameCallbacks.size() - 1;
	}

	void unsubcribeToRunFrame(int id)
	{
		runFrameCallbacks.erase(begin(runFrameCallbacks) + id);
	}
}


// Initializes the ETJump subsystems
extern "C" void ETJump_InitGame(int levelTime, int randomSeed, int restart)
{
	Printer::LogPrint(
		"--------------------------------------------------------------------------------\n"
		"Initializing ETJump subsystems\n"
		"--------------------------------------------------------------------------------\n"
	);
	runFrameCallbacks.clear();

	// each subsystem will subscribe to necessary events
	if (g_banners.integer)
	{
		InitBannerSystem();
	}

	Printer::LogPrint(
		"--------------------------------------------------------------------------------\n"
		"ETJump subsystems initialized\n"
		"--------------------------------------------------------------------------------\n"
	);
}

// Shuts down the ETJump subsystems
extern "C" void ETJump_ShutdownGame(int restart)
{
	Printer::LogPrint(
		"--------------------------------------------------------------------------------\n"
		"Shutting down the ETJump subsystems\n"
		"--------------------------------------------------------------------------------\n"
	);

	ShutdownBannerSystem();

	Printer::LogPrint(
		"--------------------------------------------------------------------------------\n"
		"ETJump subsystems shut down\n"
		"--------------------------------------------------------------------------------\n"
	);
}

extern "C" void ETJump_RunFrame(int levelTime)
{
	for (auto & callback : runFrameCallbacks)
	{
		callback(levelTime);
	}
}


