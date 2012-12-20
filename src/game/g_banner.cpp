#include <string>

using std::string;

extern "C" {
#include "g_local.h"
}

#include "g_utilities.h"

// Banner locations
enum {
    BANNER_CP,
    BANNER_BP,
    BANNER_SAY,
    BANNER_CPM
};

const int MAX_BANNERS = 5;
const int DEFAULT_BANNER_TIME = 60000;
const int MIN_BANNER_TIME = 5000;
const int MAX_BANNER_TIME = 600000;

static unsigned banner_count;
static string banners[MAX_BANNERS];

void SetBanners() {
    banner_count = 0;

    for(unsigned i = 0; i < MAX_BANNERS; i++) {
        banners[i].clear();
    }

    if(strlen(g_banner1.string)) {
        banners[banner_count++] = g_banner1.string;
    }

    if(strlen(g_banner2.string)) {
        banners[banner_count++] = g_banner2.string;
    }

    if(strlen(g_banner3.string)) {
        banners[banner_count++] = g_banner3.string;
    }

    if(strlen(g_banner4.string)) {
        banners[banner_count++] = g_banner4.string;
    }

    if(strlen(g_banner5.string)) {
        banners[banner_count++] = g_banner5.string;
    }
}

void CheckBanners() {

    if(banner_count == 0) {
        return;
    }

    static int next_banner = 0;
    static int next_banner_time = 0;

	if(next_banner_time > level.time) {
		return;
	}

	string message;
	int banner_location = BANNER_BP;
	int banner_time = DEFAULT_BANNER_TIME;

	// I don't think anyone needs to see a banner every 5 seconds
	// or once every 30 minutes. 30seconds - 10minutes.
    if(g_bannerTime.integer >= MIN_BANNER_TIME && g_bannerTime.integer <= MAX_BANNER_TIME) {
		banner_time = g_bannerTime.integer;
	}

	if( g_bannerLocation.integer != BANNER_CP &&
		g_bannerLocation.integer != BANNER_CPM &&
		g_bannerLocation.integer != BANNER_SAY &&
		g_bannerLocation.integer != BANNER_BP ) {
			banner_location = BANNER_BP;
	} else {
		banner_location = g_bannerLocation.integer;
	}

    message = banners[next_banner];

    switch(banner_location) {
    case BANNER_CP:
        CPPrintAll(message);
        break;
    case BANNER_CPM:
        CPMPrintAll(message);
        break;
    case BANNER_SAY:
        ChatPrintAll(message);
        break;
    case BANNER_BP:
        BannerPrintAll(message);
        break;
    default: 
        BannerPrintAll(message);
    }

    if(next_banner < banner_count - 1) {
        next_banner++;
    } else {
        next_banner = 0;
    }

	next_banner_time = level.time + banner_time;
}