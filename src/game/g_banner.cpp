extern "C" {
#include "g_local.h"
}

#include "g_utilities.hpp"

#include <string>
using std::string;

// Banner locations
enum {
    BANNER_CP,
    BANNER_BP,
    BANNER_SAY,
    BANNER_CPM
};

const unsigned MAX_BANNERS = 5;
const unsigned DEFAULT_BANNER_TIME = 60000;
const unsigned MIN_BANNER_TIME = 5000;
const unsigned MAX_BANNER_TIME = 600000;

static unsigned bannerCount;
static string banners[MAX_BANNERS];

void SetBanners() {
    bannerCount = 0;

    for(unsigned i = 0; i < MAX_BANNERS; i++) {
        banners[i].clear();
    }

    if(strlen(g_banner1.string)) {
        banners[bannerCount++] = g_banner1.string;
    }

    if(strlen(g_banner2.string)) {
        banners[bannerCount++] = g_banner2.string;
    }

    if(strlen(g_banner3.string)) {
        banners[bannerCount++] = g_banner3.string;
    }

    if(strlen(g_banner4.string)) {
        banners[bannerCount++] = g_banner4.string;
    }

    if(strlen(g_banner5.string)) {
        banners[bannerCount++] = g_banner5.string;
    }
}

void CheckBanners() {

    if(bannerCount == 0) {
        return;
    }

    static int nextBanner = 0;
    static int nextBannerTime = 0;

	if(nextBannerTime > level.time) {
		return;
	}

	string message;
	int bannerLocation = BANNER_BP;
	int bannerTime = DEFAULT_BANNER_TIME;

	// I don't think anyone needs to see a banner every 5 seconds
	// or once every 30 minutes. 30seconds - 10minutes.
    if(g_bannerTime.integer >= static_cast<int>(MIN_BANNER_TIME) 
        && g_bannerTime.integer <= static_cast<int>(MAX_BANNER_TIME)) {
		bannerTime = g_bannerTime.integer;
	}

	if( g_bannerLocation.integer != BANNER_CP &&
		g_bannerLocation.integer != BANNER_CPM &&
		g_bannerLocation.integer != BANNER_SAY &&
		g_bannerLocation.integer != BANNER_BP ) {
			bannerLocation = BANNER_BP;
	} else {
		bannerLocation = g_bannerLocation.integer;
	}

    message = banners[nextBanner];

    switch(bannerLocation) {
    case BANNER_CP:
        CPAll(message, false);
        break;
    case BANNER_CPM:
        CPMAll(message, false);
        break;
    case BANNER_SAY:
        ChatPrintAll(message, false);
        break;
    case BANNER_BP:
        BPAll(message, false);
        break;
    default: 
        BPAll(message, false);
    }

    G_LogPrintf("banner: %s\n", message.c_str());

    if(nextBanner < static_cast<int>(bannerCount) - 1) {
        nextBanner++;
    } else {
        nextBanner = 0;
    }

	nextBannerTime = level.time + bannerTime;
}

