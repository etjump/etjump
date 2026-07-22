/*
 * MIT License
 *
 * Copyright (c) 2026 ETJump team <zero@etjump.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "etj_upmove_meter_v2.h"
#include "etj_upmove_meter_data.h"
#include "etj_color_parser.h"
#include "etj_cvar_update_handler.h"
#include "etj_utilities.h"

namespace ETJump {
inline constexpr float UPMOVEMETER_POS_X = 8.0f;
inline constexpr float UPMOVEMETER_POS_Y = 8.0f;

UpmoveMeterV2::UpmoveMeterV2(
    const std::shared_ptr<UpmoveMeterData> &upmoveMeterData,
    const std::shared_ptr<CvarUpdateHandler> &cvarUpdate)
    : upmoveMeterData(upmoveMeterData), cvarUpdate(cvarUpdate) {
  cgame.utils.colorParser->parseColorString(etj_upmoveMeterGraphColor.string,
                                            graph.colorBg);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphOnGroundColor.string, graph.colorOnGround);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphPreJumpColor.string, graph.colorPreJump);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphPostJumpColor.string, graph.colorPostJump);
  cgame.utils.colorParser->parseColorString(
      etj_upmoveMeterGraphOutlineColor.string, graph.colorOutline);

  cgame.utils.colorParser->parseColorString(etj_upmoveMeterTextColor.string,
                                            colorText);

  setTextSize(etj_upmoveMeterTextSize);
  startListeners();
}

UpmoveMeterV2::~UpmoveMeterV2() {
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphOnGroundColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphPreJumpColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphPostJumpColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterGraphOutlineColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterTextColor);
  cvarUpdate->unsubscribe(&etj_upmoveMeterTextSize);
}

void UpmoveMeterV2::startListeners() {
  cvarUpdate->subscribe(
      &etj_upmoveMeterGraphColor, [this](const vmCvar_t *cvar) {
        cgame.utils.colorParser->parseColorString(cvar->string, graph.colorBg);
      });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphOnGroundColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, graph.colorOnGround);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphPreJumpColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, graph.colorPreJump);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphPostJumpColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, graph.colorPostJump);
                        });

  cvarUpdate->subscribe(&etj_upmoveMeterGraphOutlineColor,
                        [this](const vmCvar_t *cvar) {
                          cgame.utils.colorParser->parseColorString(
                              cvar->string, graph.colorOutline);
                        });

  cvarUpdate->subscribe(
      &etj_upmoveMeterTextColor, [this](const vmCvar_t *cvar) {
        cgame.utils.colorParser->parseColorString(cvar->string, colorText);
      });

  cvarUpdate->subscribe(&etj_upmoveMeterTextSize,
                        [this](const vmCvar_t *cvar) { setTextSize(*cvar); });
}

void UpmoveMeterV2::setTextSize(const vmCvar_t &cvar) {
  textSize = CvarValueParser::parse<CvarValue::Size>(cvar);
  textSize.x *= 0.1f;
  textSize.y *= 0.1f;

  textHeightOffset = static_cast<float>(CG_Text_Height_Ext(
                         "0", textSize.y, 0, &cgs.media.limboFont1)) *
                     0.5f;
}

bool UpmoveMeterV2::beforeRender() {
  if (canSkipDraw()) {
    return false;
  }

  const auto &s = upmoveMeterData->getState();

  graph.absMaxDelay = std::min(std::clamp(etj_upmoveMeterMaxDelay.integer, 0,
                                          UpmoveMeterData::MAX_UPMOVE_TIME),
                               UpmoveMeterData::MAX_UPMOVE_TIME);

  graph.preDelay =
      std::clamp(s.preDelay, -graph.absMaxDelay, graph.absMaxDelay);
  graph.postDelay = std::min(s.postDelay, graph.absMaxDelay);
  graph.fullDelay = s.fullDelay;

  graph.rect.x =
      std::clamp(UPMOVEMETER_POS_X + etj_upmoveMeterGraphX.value, 0.0f, 640.0f);
  graph.rect.y = std::clamp(UPMOVEMETER_POS_Y + etj_upmoveMeterGraphY.value,
                            0.0f, static_cast<float>(SCREEN_HEIGHT));
  graph.rect.w = etj_upmoveMeterGraphW.value;
  graph.rect.h = etj_upmoveMeterGraphH.value;

  graph.upHeight = static_cast<float>(graph.postDelay) /
                   std::max(static_cast<float>(graph.absMaxDelay), 1.0f);
  graph.upHeight *= graph.rect.h * 0.5f;

  graph.downHeight = static_cast<float>(std::abs(graph.preDelay)) /
                     std::max(static_cast<float>(graph.absMaxDelay), 1.0f);
  graph.downHeight *= graph.rect.h * 0.5f;

  textX = std::clamp(graph.rect.x + graph.rect.w + etj_upmoveMeterTextX.value,
                     0.0f, 640.0f);
  textH = etj_upmoveMeterTextH.value;

  // NOTE: apply widescreen adjustment to 'rect.x' only after 'textX'
  // has been calculated as well
  ETJump_AdjustPosition(&graph.rect.x);
  ETJump_AdjustPosition(&textX);

  textStyle = etj_upmoveMeterTextShadow.integer ? ITEM_TEXTSTYLE_SHADOWED
                                                : ITEM_TEXTSTYLE_NORMAL;

  return true;
}

void UpmoveMeterV2::render() const {
  const float graphHH = graph.rect.h * 0.5f;
  const float graphM = graph.rect.y + graphHH;

  // graph
  if (etj_drawUpmoveMeter.integer & 1) {
    CG_FillRect(graph.rect, graph.colorBg);

    CG_FillRect(graph.rect.x, graph.rect.y + graphHH, graph.rect.w,
                graph.downHeight,
                graph.preDelay < 0 ? graph.colorOnGround : graph.colorPreJump);
    CG_FillRect(graph.rect.x, graphM - graph.upHeight, graph.rect.w,
                graph.upHeight, graph.colorPostJump);

    CG_DrawRect(graph.rect, etj_upmoveMeterGraphOutlineW.value,
                graph.colorOutline);
  }

  // text
  if (etj_drawUpmoveMeter.integer & 2) {
    CG_Text_Paint_Ext(textX, graphM - textH + textHeightOffset, textSize.x,
                      textSize.y, colorText, std::to_string(graph.postDelay), 0,
                      0, textStyle, &cgs.media.limboFont1);

    CG_Text_Paint_Ext(textX, graphM + textHeightOffset, textSize.x, textSize.y,
                      colorText, std::to_string(graph.fullDelay), 0, 0,
                      textStyle, &cgs.media.limboFont1);

    CG_Text_Paint_Ext(textX, graphM + textH + textHeightOffset, textSize.x,
                      textSize.y, colorText, std::to_string(graph.preDelay), 0,
                      0, textStyle, &cgs.media.limboFont1);
  }
}

bool UpmoveMeterV2::canSkipDraw() {
  if (!etj_drawUpmoveMeter.integer) {
    return true;
  }

  if (cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
    return true;
  }

  if (showingScores()) {
    return true;
  }

  return false;
}
} // namespace ETJump
