#include "etj_quick_follow_drawable.h"
#include "../game/etj_string_utilities.h"

char *BindingFromName(const char *cvar);

ETJump::QuickFollowDrawer::QuickFollowDrawer() {}

void ETJump::QuickFollowDrawer::beforeRender() {}

void ETJump::QuickFollowDrawer::render() const
{
	if (canSkipDraw())
	{
		return;
	}

	float *color = CG_FadeColor(cg.crosshairClientTime, 1000);
	if (!color)
	{
		return;
	}

	auto binding = BindingFromName("+activate");
	auto hintText = cgs.clientinfo[cg.crosshairClientNum].specLocked ? "^9Speclocked" : stringFormat("^9[%s] to follow", binding);
	float w = DrawStringWidth(cgs.clientinfo[cg.crosshairClientNum].name, 0.23f);
	auto offx = SCREEN_CENTER_X - w / 2;
	DrawString(offx, 182 + 10, 0.16f, 0.16f, color, qfalse, hintText.c_str(), 0, ITEM_TEXTSTYLE_SHADOWED);
}

bool ETJump::QuickFollowDrawer::canSkipDraw() const
{
	if (etj_quickFollow.integer < 2 || cg.crosshairClientNum > MAX_CLIENTS)
	{
		return true;
	}
	if (cg.showScores || cg.scoreFadeTime + FADE_TIME > cg.time)
	{
		return true;
	}
	if (cgs.clientinfo[cg.crosshairClientNum].hideMe)
	{
		return true;
	}
	return false;
}
