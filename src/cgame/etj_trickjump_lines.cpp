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

#include "etj_trickjump_lines.h"
#include "json/json.h"
#include <sstream>
#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <cstring>
#include <limits>

#include "cg_local.h"

static const char *EnumStrings[] = {"mapper", "loaded", "recorded"};
const char *getTextForEnum(int enumVal) { return EnumStrings[enumVal]; }

TrickjumpLines::TrickjumpLines()
    : _nextRecording(1), _nextAddTime(0), _currentRouteToRender(-1) {
  this->_recording = false;
  this->_jumpRelease = true;
  this->_currentRotation.init();
  this->_debugVerbose = false;

  // Create a map of possible color for TJL.
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "white", {255, 255, 255, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "black", {0, 0, 0, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "red", {255, 0, 0, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "green", {0, 255, 0, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "blue", {0, 0, 255, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "yellow", {255, 255, 0, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "magenta", {128, 0, 128, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "cyan", {0, 128, 128, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "orange", {128, 128, 0, 255}));
  colorMap.insert(std::pair<std::string, std::vector<unsigned char>>(
      "speed", {0, 0, 0, 0}));
}

TrickjumpLines::~TrickjumpLines() {}

// Create simple function to return cvar.
bool TrickjumpLines::isEnableLine() { return this->_enableLine; }
bool TrickjumpLines::isEnableMarker() { return this->_enableMarker; }

void TrickjumpLines::record(const char *name) {
  if (isRecording()) {
    CG_Printf("You are already recording. \n");
    return;
  }

  // TODO: Check if user is in spec ? (Xis)
  Route route;
  route.name =
      name != nullptr ? name : "tjl_" + std::to_string(_nextRecording++);

  if (getRoutePositionByName(route.name.c_str()) > -1) {
    if (name == nullptr) {
      while (true) {
        route.name = "tjl_" + std::to_string(_nextRecording++);
        if (getRoutePositionByName(route.name.c_str()) == -1) {
          CG_Printf("Route will be created "
                    "with number : %d \n",
                    _nextRecording);
          break;
        }
      }
    } else {
      CG_Printf("This route already exist. Please "
                "provide a unique name or use "
                "/tjl_overwriterecording to over-write "
                "a route. \n");
      _nextRecording--;
      return;
    }
  }

  route.width = LINE_WIDTH;
  route.status = routeStatus::record;
  if (isDebug()) {
    CG_Printf("Recording : %s\n", route.name.c_str());
  }
  _currentRoute = std::move(route);

  // Have to clear currentTrail.
  _currentTrail.clear();
  _recording = true;
}

void TrickjumpLines::overwriteRecording(const char *name) {
  if (isRecording()) {
    CG_Printf("You are already recording. \n");
    return;
  }

  if (name == nullptr) {
    CG_Printf("You need to pass a route name by argument to "
              "over-write a "
              "route. Use command /tjl_listroute to get name. \n");
    return;
  }

  const int z = getRoutePositionByName(name);
  if (z == -1) {
    CG_Printf("No route exists with this name. \n");
    return;
  } else {
    if (_routes[z].status == routeStatus::map) {
      CG_Printf("You can't overwrite this route. "
                "Mapper TJL are read-only. \n");
      return;
    }
  }

  Route route = _routes[z];
  CG_Printf("Over-writing : %s\n", route.name.c_str());
  _currentRoute = std::move(route);

  // Have to clear currentTrail.
  _currentTrail.clear();
  _recording = true;
}

void TrickjumpLines::addPosition(vec3_t pos) {
  if (_recording) {
    // Check if player is currently in air
    if ((cg.predictedPlayerState.stats[STAT_USERCMD_MOVE] & UMOVE_UP) &&
        (_jumpRelease)) {
      // Player press jump key.
      _jumpRelease = false;

      // Make a copy of currentTrail.
      std::vector<Node> trail;
      trail = std::move(_currentTrail);
      _currentTrail.clear();
      _currentTrail.push_back(trail[trail.size() - 1]);

      // Add trail to route.
      _currentRoute.trails.push_back(trail);
    } else if ((cg.predictedPlayerState.stats[STAT_USERCMD_MOVE] & UMOVE_UP) &&
               (!_jumpRelease)) {
      // Still pressing the jump key.
      _jumpRelease = false;
    } else {
      // Jump key is release.
      _jumpRelease = true;
    }

    // Check if time to add another points in list.
    if (cg.time > _nextAddTime) {
      Node cNode;
      vec3_t vec;

      // Copy C position into Trickjump(C++) vec.
      vec[0] = pos[0];
      vec[1] = pos[1];

      // TODO: (XIS) make a cvar with this hardcoded.
      vec[2] = pos[2] - 24;

      // Copy into Node struct.
      VectorCopy(vec, cNode.coor);

      // Add speed
      cNode.speed = sqrt(cg.predictedPlayerState.velocity[0] *
                             cg.predictedPlayerState.velocity[0] +
                         cg.predictedPlayerState.velocity[1] *
                             cg.predictedPlayerState.velocity[1]);

      // TODO: (XIS) add this ?, if player stay on same
      // spot too long. if
      // (_currentTrail[_currentTrail.size() -1].coor !=
      // cNode.coor)

      // Add node to the current trail.
      _currentTrail.push_back(cNode);

      _nextAddTime = cg.time + 50; // 20 times a sec // FRAMETIME =
                                   // 10 times a sec.
    }
  }
}

void TrickjumpLines::stopRecord() {
  if (!isRecording()) {
    return;
  }

  std::vector<Node> trail;
  trail = std::move(_currentTrail);
  _currentTrail.clear();
  _currentRoute.trails.push_back(trail);
  _recording = false;
  _routes.push_back(_currentRoute);

  CG_Printf("Stopped recording: %s\n", _currentRoute.name.c_str());
  CG_Printf("Total of trail in this route : %d\n",
            static_cast<int>(_currentRoute.trails.size()));

  setCurrentRouteToRender(countRoute() - 1);
  displayCurrentRoute(getCurrentRouteToRender());
}

void TrickjumpLines::displayCurrentRoute(int x) {
  // Loop on every trail into the route.
  const int nbTrails = _routes[x].trails.size();

  // Get min and max speed of the current jump.
  float minSpeed = std::numeric_limits<float>::max();
  float maxSpeed = 0;

  for (auto i = 0; i < nbTrails; ++i) {
    std::vector<Node> cTrail = _routes[x].trails[i];
    const int nbPoints = cTrail.size();
    for (int j = 0; j < nbPoints; ++j) {
      if (cTrail[j].speed < minSpeed)
        minSpeed = cTrail[j].speed;

      if (cTrail[j].speed > maxSpeed)
        maxSpeed = cTrail[j].speed;
    }
  }

  for (auto i = 0; i < nbTrails; ++i) {
    // Get current trail into tmp var.
    std::vector<Node> cTrail = _routes[x].trails[i];
    const int nbPoints = cTrail.size();

    // Add the bezier curve of this trail.
    if (isEnableLine()) {
      // addTrickjumpRecursiveBezier(cTrail, _blue,
      // _routes[x].width, 150);
      const std::string tmp = etj_tjlLineColor.string;
      if (tmp == "speed" || tmp == "Speed") {
        addTrickjumpLinesColor(cTrail, minSpeed, maxSpeed, _routes[x].width);
      } else {
        vec4_c colorLine;
        colorLine[0] = colorMap[etj_tjlLineColor.string][0];
        colorLine[1] = colorMap[etj_tjlLineColor.string][1];
        colorLine[2] = colorMap[etj_tjlLineColor.string][2];
        colorLine[3] = colorMap[etj_tjlLineColor.string][3];
        addTrickjumpLines(cTrail, colorLine, _routes[x].width);
      }
    }

    // Add curve indicator.
    if (isEnableMarker()) {
      vec3_t start, end;
      VectorCopy(cTrail[0].coor, start);
      VectorCopy(cTrail[nbPoints - 1].coor, end);

      std::ostringstream ss;
      ss << (cTrail[0].speed);
      const std::string speedStr(ss.str());
      vec4_c colorEndMarker, colorMarker;
      colorEndMarker[0] = colorMap[etj_tjlMarkerEndColor.string][0];
      colorEndMarker[1] = colorMap[etj_tjlMarkerEndColor.string][1];
      colorEndMarker[2] = colorMap[etj_tjlMarkerEndColor.string][2];
      colorEndMarker[3] = colorMap[etj_tjlMarkerEndColor.string][3];
      colorMarker[0] = colorMap[etj_tjlMarkerColor.string][0];
      colorMarker[1] = colorMap[etj_tjlMarkerColor.string][1];
      colorMarker[2] = colorMap[etj_tjlMarkerColor.string][2];
      colorMarker[3] = colorMap[etj_tjlMarkerColor.string][3];

      // check if only 1 trail.
      if (nbTrails == 1) {
        addJumpIndicator(start, colorEndMarker, 10.0);
        addJumpIndicator(end, colorEndMarker, 10.0);
      }
      // Check if it is the first curve of the route.
      else if (i == 0) {
        addJumpIndicator(start, colorEndMarker, 10.0);
      }
      // Check if it is the last curve of the route.
      else if (i == static_cast<int>(_routes[x].trails.size()) - 1) {
        addJumpIndicator(start, colorMarker, 10.0);
        addJumpIndicator(end, colorEndMarker, 10.0);
        // drawFloatingText(speedStr, start,
        // _red, 3); // TODO: Not working yet.
        // (Xis)
      }
      // If any another curve of the route.
      else {
        addJumpIndicator(start, colorMarker, 10.0);
        // drawFloatingText(speedStr, start,
        // _red, 3); // TODO: Not working yet.
        // (Xis)
      }
    }
  }
}

// gcd_ui, use in Binomial coefficient function.
unsigned long TrickjumpLines::gcd_ui(unsigned long x, unsigned long y) {
  unsigned long t;
  if (y < x) {
    t = x;
    x = y;
    y = t;
  }
  while (y > 0) {
    t = y;
    y = x % y;
    x = t; /* y1 <- x0 % y0 ; x1 <- y0 */
  }
  return x;
}

// Compute the binomial coefficient base on k in n
unsigned long TrickjumpLines::binomial(unsigned long n, unsigned long k) {
  unsigned long d, g, r = 1;

  // Trivial case.
  if (k == 0)
    return 1;
  if (k == 1)
    return n;
  if (k >= n)
    return (k == n);
  if (k > n / 2)
    k = n - k;

  for (d = 1; d <= k; ++d) {
    if (r >= ULONG_MAX / n) /* Possible overflow */
    {
      unsigned long nr, dr; /* reduced numerator / denominator */
      g = gcd_ui(n, d);
      nr = n / g;
      dr = d / g;
      g = gcd_ui(r, dr);
      r = r / g;
      dr = dr / g;
      if (r >= ULONG_MAX / nr)
        return 0; /* Unavoidable overflow */
      r *= nr;
      r /= dr;
      n--;
    } else {
      r *= n--;
      r /= d;
    }
  }
  return r;
}

// Draw all 4 vertices to make the quad (line) with the width and color define
// by user.
void TrickjumpLines::draw4VertexLine(vec3_t start, vec3_t end, float width,
                                     vec4_c color) {
  // Draw a small line between each start/end
  polyVert_t verts[4];
  vec3_t up, pDraw;
  int cIdx = 0;

  // Obtain Up vector, base on player location
  GetPerpendicularViewVector(cg.refdef_current->vieworg, start, end, up);

  // 1 vertex
  VectorMA(start, 0.5 * width, up, pDraw);
  VectorCopy(pDraw, verts[cIdx].xyz);
  verts[cIdx].st[0] = 0;
  verts[cIdx].st[1] = 1.0;

  for (int k = 0; k < 4; ++k)
    verts[cIdx].modulate[k] = (unsigned char)(color[k]);

  ++cIdx;

  // 2 vertex
  VectorMA(pDraw, -1.0 * width, up, pDraw);
  VectorCopy(pDraw, verts[cIdx].xyz);
  verts[cIdx].st[0] = 0;
  verts[cIdx].st[1] = 0;

  for (int k = 0; k < 4; ++k)
    verts[cIdx].modulate[k] = (unsigned char)(color[k]);

  ++cIdx;

  // 3 vertex
  VectorMA(end, -0.5 * width, up, pDraw);
  VectorCopy(pDraw, verts[cIdx].xyz);
  verts[cIdx].st[0] = 1.0;
  verts[cIdx].st[1] = 0.0;

  for (int k = 0; k < 4; ++k)
    verts[cIdx].modulate[k] = (unsigned char)(color[k]);

  ++cIdx;
  // 4 vertex
  VectorMA(pDraw, width, up, pDraw);
  VectorCopy(pDraw, verts[cIdx].xyz);
  verts[cIdx].st[0] = 1.0;
  verts[cIdx].st[1] = 1.0;

  for (int k = 0; k < 4; ++k)
    verts[cIdx].modulate[k] = (unsigned char)(color[k]);

  // Add vertices to scene as quad.
  trap_R_AddPolyToScene(cgs.media.railCoreShader, 4, verts);

  return;
}

void TrickjumpLines::draw4VertexLine2Color(vec3_t start, vec3_t end,
                                           float width, vec4_c colorStart,
                                           vec4_c colorEnd) {
  // Draw a small line between each start/end
  polyVert_t verts[4];
  vec3_t up, pDraw;
  int cIdx = 0;

  // Obtain Up vector, base on player location
  GetPerpendicularViewVector(cg.refdef_current->vieworg, start, end, up);

  // 1 vertex
  VectorMA(start, 0.5 * width, up, pDraw);
  VectorCopy(pDraw, verts[cIdx].xyz);
  verts[cIdx].st[0] = 0;
  verts[cIdx].st[1] = 1.0;

  for (int k = 0; k < 4; ++k)
    verts[cIdx].modulate[k] = (unsigned char)(colorStart[k]);

  ++cIdx;

  // 2 vertex
  VectorMA(pDraw, -1.0 * width, up, pDraw);
  VectorCopy(pDraw, verts[cIdx].xyz);
  verts[cIdx].st[0] = 0;
  verts[cIdx].st[1] = 0;

  for (int k = 0; k < 4; ++k)
    verts[cIdx].modulate[k] = (unsigned char)(colorStart[k]);

  ++cIdx;

  // 3 vertex
  VectorMA(end, -0.5 * width, up, pDraw);
  VectorCopy(pDraw, verts[cIdx].xyz);
  verts[cIdx].st[0] = 1.0;
  verts[cIdx].st[1] = 0.0;

  for (int k = 0; k < 4; ++k)
    verts[cIdx].modulate[k] = (unsigned char)(colorEnd[k]);

  ++cIdx;
  // 4 vertex
  VectorMA(pDraw, width, up, pDraw);
  VectorCopy(pDraw, verts[cIdx].xyz);
  verts[cIdx].st[0] = 1.0;
  verts[cIdx].st[1] = 1.0;

  for (int k = 0; k < 4; ++k)
    verts[cIdx].modulate[k] = (unsigned char)(colorEnd[k]);

  // Add vertices to scene as quad.
  trap_R_AddPolyToScene(cgs.media.railCoreShader, 4, verts);

  return;
}

// Compute the bezier's curves base on recursive function (so N-degree).
// The function is able to draw the line between start and end point, plus any
// number of controls points between them. Just by passing an array of vec3_t
// where points[0] = start and points[end] = end. Color is an array of size 4,
// which contain, rgb-a color. Width is the width for the line and nbDivision is
// the number of division process during bezier curves Less divison = more
// straight line and more division = better curve (longer processing)
void TrickjumpLines::addTrickjumpRecursiveBezier(std::vector<Node> points,
                                                 vec4_c color, float width,
                                                 int nbDivision) {
  static int nextPrintTime = 0;
  vec3_t zeros = {0.0, 0.0, 0.0};
  const int n = points.size();

  if (n < 2) {
    if (nextPrintTime < cg.time) {
      if (isDebug()) {
        CG_Printf("Exit Bezier drawing, not "
                  "enought points. \n");
      }
      nextPrintTime = cg.time + 1000;
    }
    return;
  }

  if (n > nbDivision) {
    nbDivision = n;
  }

  // Hold previous point on each iter (start)
  vec3_t prev;
  VectorCopy(points[0].coor, prev);

  // Divide bezier line into nbDivision points
  for (int i = 0; i < nbDivision; ++i) {
    // Compute bezier cubic equation values.
    const float t = i / (float)nbDivision;
    const float u = 1 - t;

    // Computed points variable
    vec3_t p, finalp, tmp;

    // To get a full line base on start/end plus X control
    // points for a total of N = X + 2 points.
    // https://en.wikipedia.org/wiki/B%C3%A9zier_curve see
    // Explicit definition of Recursive definition Compute the
    // recursive equation into point p.

    // Do equation on P0 (start)
    VectorMA(zeros, pow(u, n), points[0].coor, p);

    // Do equation on P1 to Pn-1 (Control points).
    for (int l = 1; l < n; ++l) {
      VectorMA(zeros, binomial(n, l) * pow(u, n - l) * pow(t, l),
               points[l].coor, tmp);
      VectorAdd(p, tmp, p);
    }

    // Do equation on Pn (end) into finalp
    VectorMA(zeros, pow(t, n), points[n - 1].coor, tmp);
    VectorAdd(p, tmp, finalp);

    // Draw line between previous and finalp with
    // trap_R_AddPolyToScene.
    draw4VertexLine(prev, finalp, width, color);

    // Copy current point into prev for next iteration.
    VectorCopy(finalp, prev);
  }
  return;
}

void TrickjumpLines::addTrickjumpLines(std::vector<Node> points, vec4_c color,
                                       float width) {
  static int nextPrintTime = 0;
  const int n = points.size();

  if (n < 2) {
    if (nextPrintTime < cg.time) {
      nextPrintTime = cg.time + 1000;
    }
    return;
  }

  for (int i = 0; i < n - 1; ++i) {
    vec3_t s, e;
    VectorCopy(points[i].coor, s);
    VectorCopy(points[i + 1].coor, e);

    // Draw line between previous and finalp with
    // trap_R_AddPolyToScene.
    draw4VertexLine(s, e, width, color);
  }

  return;
}

void TrickjumpLines::addTrickjumpLinesColor(std::vector<Node> points,
                                            float minSpeed, float maxSpeed,
                                            float width) {
  static int nextPrintTime = 0;
  const int n = points.size();

  if (n < 2) {
    if (nextPrintTime < cg.time) {
      if (isDebug()) {
        CG_Printf("Exit line drawing, not "
                  "enought points. \n");
      }
      nextPrintTime = cg.time + 1000;
    }
    return;
  }

  for (int i = 0; i < n - 1; ++i) {
    vec3_t s, e;
    VectorCopy(points[i].coor, s);
    VectorCopy(points[i + 1].coor, e);

    // Obtain color base on speed.
    vec3_t color1, color2;
    computeColorForNode(maxSpeed, minSpeed, points[i].speed, color1);
    computeColorForNode(maxSpeed, minSpeed, points[i + 1].speed, color2);

    vec4_c c1, c2;
    c1[0] = static_cast<unsigned char>(color1[0]);
    c1[1] = static_cast<unsigned char>(color1[1]);
    c1[2] = static_cast<unsigned char>(color1[2]);
    c1[3] = static_cast<unsigned char>(255);

    c2[0] = static_cast<unsigned char>(color2[0]);
    c2[1] = static_cast<unsigned char>(color2[1]);
    c2[2] = static_cast<unsigned char>(color2[2]);
    c2[3] = static_cast<unsigned char>(255);

    // Draw line between previous and finalp with
    // trap_R_AddPolyToScene.
    draw4VertexLine2Color(s, e, width, c1, c2);
  }
}

bool TrickjumpLines::loadedRoutes(const char *loadname) {
  for (auto &route : _routes) {
    if (loadname == nullptr) {
      CG_Printf("You request to load mapper TJL.\n");
      if (route.status == routeStatus::map)
        return true;
    } else {
      std::string tmp = loadname;
      if (route.filename == tmp)
        return true;
    }
  }

  return false;
}

void TrickjumpLines::loadRoutes(const char *loadname) {
  std::string map;
  fileHandle_t f = 0;
  routeStatus loadStatus;

  // Check if already loaded
  if (loadedRoutes(loadname)) {
    CG_Printf("This file is already loaded : %s.\n", loadname);
    return;
  }
  // Always load mapper map, and
  if (loadname == nullptr) {
    map = (std::string("tjllines/mapper/") + cgs.rawmapname +
           std::string(".tjl"));
    loadStatus = routeStatus::map;
    // CG_Printf("Will load mapper TJL for map : %s.\n",
    // cgs.rawmapname);
  } else if (loadname != nullptr) {
    map = (std::string("tjllines/") + cgs.rawmapname + std::string("/") +
           loadname + std::string(".tjl"));
    loadStatus = routeStatus::load;

    if (trap_FS_FOpenFile(map.c_str(), &f, FS_READ) > 0) {
      // CG_Printf("Will load local TJL file : %s.\n",
      // loadname);
    }
  }

  const int len = trap_FS_FOpenFile(map.c_str(), &f, FS_READ);
  if (len <= 0) {
    // CG_Printf("File not found : %s.\n", map.c_str());
    return;
  }

  std::shared_ptr<char> buf(new char[len + 1], [](char *p) { delete[] p; });
  trap_FS_Read(buf.get(), len, f);
  buf.get()[len] = 0;

  std::string json(buf.get());
  Json::Value root;
  Json::Reader reader;

  if (!reader.parse(json, root)) {
    CG_Printf("Json parser error in file: %s\n", map.c_str());
    return;
  }

  try {
    // Loop on each route (tjl)
    for (auto &i : root) {
      Route loadRoute;
      if (loadname != nullptr) {
        loadRoute.filename = loadname;
      }

      loadRoute.name = i["name"].asString();
      loadRoute.width = i["width"].asFloat();
      loadRoute.status = loadStatus;

      Json::Value colorValue = i["color"];
      for (int j = 0; j < static_cast<int>(colorValue.size()); ++j) {
        loadRoute.color[j] =
            (unsigned char)Q_atoi(colorValue[j].asString().c_str());
      }

      // Loop on each trail in a route (tjl)
      Json::Value trailsValue = i["trails"];
      std::vector<std::vector<Node>> routeVec;
      for (auto &j : trailsValue) {
        // Loop on each node in a trail.
        std::vector<Node> trailVec;
        for (auto &k : j) {
          Node loadNode{};
          Json::Value coorValue = k["coordinates"];
          for (int l = 0; l < static_cast<int>(coorValue.size()); ++l) {
            loadNode.coor[l] = Q_atof(coorValue[l].asString().c_str());
          }
          loadNode.speed = k["speed"].asFloat();
          trailVec.push_back(loadNode); // Add node to trail.
        }
        routeVec.push_back(trailVec); // Add trail to route.
      }
      loadRoute.trails = std::move(routeVec);
      _routes.push_back(loadRoute); // Add route to object
    }
  } catch (...) {
    CG_Printf("There was a read error in %s parser\n", map.c_str());
    return;
  }
}

void TrickjumpLines::saveRoutes(const char *savename) {
  // TODO (xis) : if file name already exist, overwrite?
  fileHandle_t f = 0;
  if (trap_FS_FOpenFile((std::string("tjllines/") + cgs.rawmapname +
                         std::string("/") + savename + std::string(".tjl"))
                            .c_str(),
                        &f, FS_READ) > 0) {
    CG_Printf("This file already exists, cannot save.\n");
    return;
  }

  Json::Value root = Json::arrayValue;
  for (auto &route : _routes) {
    // if (route.status == routeStatus::record || route.status
    // == routeStatus::load)
    if (route.status == routeStatus::record) {
      Json::Value jsonRoute;
      jsonRoute["name"] = route.name;
      // TODO: based on a cvar or something? No magic
      // values, yay! (See trickjumplines::recording for
      // frame information)
      jsonRoute["nodes per second"] = 10;
      jsonRoute["color"] = Json::arrayValue;
      for (auto i = 0; i < 4; ++i) {
        jsonRoute["color"].append(std::to_string(route.color[i]));
      }
      jsonRoute["width"] = route.width;
      jsonRoute["trails"] = Json::arrayValue;
      for (auto &trail : route.trails) {
        Json::Value jsonTrail = Json::arrayValue;
        for (auto &node : trail) {
          Json::Value jsonNode;
          jsonNode["speed"] = node.speed;
          jsonNode["coordinates"] = Json::arrayValue;
          for (auto &axis : node.coor) {
            jsonNode["coordinat"
                     "es"]
                .append(std::to_string(axis));
          }
          jsonTrail.append(jsonNode);
        }
        jsonRoute["trails"].append(jsonTrail);
      }
      root.append(jsonRoute);
    }
  }

  if (trap_FS_FOpenFile((std::string("tjllines/") + cgs.rawmapname +
                         std::string("/") + savename + std::string(".tjl"))
                            .c_str(),
                        &f, FS_WRITE) < 0) {
    throw "ERROR: couldn't open file for saving tjlines";
  }

  auto writer = Json::FastWriter();
  std::stringstream ss;
  auto str = writer.write(root);
  trap_FS_Write(str.c_str(), str.length(), f);
  trap_FS_FCloseFile(f);
  f = 0;
}

// This is a top face with sparkParticleShader
void TrickjumpLines::addJumpIndicator(vec3_t point, vec4_c color,
                                      float quadSize) {
  const vec3_t mins = {-quadSize, -quadSize, 0.0};
  const vec3_t maxs = {quadSize, quadSize, 0.0};

  // get the extents (size)
  const float extx = maxs[0] - mins[0];
  const float exty = maxs[1] - mins[1];
  const float extz = maxs[2] - mins[2];
  polyVert_t verts[4];
  vec3_t corners[8];

  // set the polygon's texture coordinates
  verts[0].st[0] = 0;
  verts[0].st[1] = 0;
  verts[1].st[0] = 0;
  verts[1].st[1] = 1;
  verts[2].st[0] = 1;
  verts[2].st[1] = 1;
  verts[3].st[0] = 1;
  verts[3].st[1] = 0;

  for (int i = 0; i < 4; ++i) {
    for (int k = 0; k < 4; ++k) {
      verts[i].modulate[k] = (unsigned char)color[k];
    }
  }

  VectorAdd(point, maxs, corners[3]);

  VectorCopy(corners[3], corners[2]);
  const vec3_t tmpx = {-extx, 0.0, 0.0};
  VectorAdd(corners[2], tmpx, corners[2]);

  VectorCopy(corners[2], corners[1]);
  const vec3_t tmpy = {0.0, -exty, 0.0};
  VectorAdd(corners[1], tmpy, corners[1]);

  VectorCopy(corners[1], corners[0]);
  const vec3_t tmpx2 = {extx, 0.0, 0.0};
  VectorAdd(corners[0], tmpx2, corners[0]);

  const vec3_t tmpz = {0.0, 0.0, -extz};
  for (int i = 0; i < 4; ++i) {
    VectorCopy(corners[i], corners[i + 4]);
    VectorAdd(corners[i], tmpz, corners[i]);
  }

  // top
  VectorCopy(corners[0], verts[0].xyz);
  VectorCopy(corners[1], verts[1].xyz);
  VectorCopy(corners[2], verts[2].xyz);
  VectorCopy(corners[3], verts[3].xyz);
  trap_R_AddPolyToScene(cgs.media.sparkParticleShader, 4, verts);
}

void TrickjumpLines::listRoutes() {
  CG_Printf("All routes names (and their number) currently loaded. \n");

  // Display name of the route with the associate status.
  int id = 0;
  for (const auto &route : _routes) {
    if (route.status == routeStatus::map) {
      CG_Printf("%s (# %d) with status : %s \n", (route.name).c_str(), id,
                getTextForEnum(route.status));
    } else {
      CG_Printf("%s (# %d) with status : %s \n",
                (route.filename + std::string("_") + route.name).c_str(), id,
                getTextForEnum(route.status));
    }
    id++;
  }
}

void TrickjumpLines::displayByName(const char *name) {
  if (name == nullptr) {
    CG_Printf("You need to pass a route name by argument. Use "
              "command "
              "/tjl_listroute to get name. \n");
    return;
  }

  const std::string tmp = name;
  const int z = getRoutePositionByName(name);

  if (z > -1) {
    // Set current rendered route to the index
    setCurrentRouteToRender(z);

    if (isDebug()) {
      CG_Printf("Will display route with name : %s \n", name);
    }

    // Call display function.
    displayCurrentRoute(getCurrentRouteToRender());
    return;
  }

  CG_Printf("No route with name : %s has been found. \n", name);
  return;
}

void TrickjumpLines::displayNearestRoutes() {
  // Check if their any route in the struct.
  if (_routes.size() == 0)
    return;

  // Obtain player position
  vec3_t p;
  VectorCopy(cg.predictedPlayerState.origin, p);

  std::vector<Route> routeCopy = _routes;

  // Sort the _routes with a lambda function that taking p (position of
  // player) as search param Check if player is near by a start point of
  // a route.
  std::sort(begin(routeCopy), end(routeCopy),
            [&](const Route &lhs, const Route &rhs) {
              return euclideanDist(p, lhs.trails[0][0].coor) <
                     euclideanDist(p, rhs.trails[0][0].coor);
            });

  Route nearestStart = routeCopy[0];

  // Check if player is near by a end point of a route.
  std::sort(begin(routeCopy), end(routeCopy),
            [&](const Route &lhs, const Route &rhs) {
              const int endLTrail = lhs.trails.size() - 1;
              const int endLNode = lhs.trails[endLTrail].size() - 1;
              const int endRTrail = rhs.trails.size() - 1;
              const int endRNode = rhs.trails[endRTrail].size() - 1;

              return euclideanDist(p, lhs.trails[endLTrail][endLNode].coor) <
                     euclideanDist(p, rhs.trails[endRTrail][endRNode].coor);
            });

  Route nearestEnd = routeCopy[0];
  const int endTrail = nearestEnd.trails.size() - 1;
  const int endNode = nearestEnd.trails[endTrail].size() - 1;

  // Check if nearest start route is nearest compare to the nearest end
  // route.
  if (euclideanDist(p, nearestStart.trails[0][0].coor) <
      euclideanDist(p, nearestEnd.trails[endTrail][endNode].coor)) {
    if (nearestStart.status == routeStatus::map) {
      return displayByName(nearestStart.name.c_str()); // Display the route by
                                                       // its name.
    } else {
      return displayByName(
          (nearestStart.filename + std::string("_") + nearestStart.name)
              .c_str()); // Display the route by its
                         // name.
    }
  } else {
    if (nearestEnd.status == routeStatus::map) {
      return displayByName(nearestEnd.name.c_str()); // Display the route
                                                     // by its name.
    } else {
      return displayByName(
          (nearestEnd.filename + std::string("_") + nearestStart.name)
              .c_str()); // Display the route by its
                         // name.
    }
  }
}

void TrickjumpLines::renameRoute(const char *oldName, const char *newName) {
  if (oldName == nullptr || newName == nullptr) {
    CG_Printf("You need to pass an existing route name by "
              "argument and the new "
              "route name. Use command /tjl_listroute to get "
              "name. \n");
    return;
  }

  if (std::strcmp(newName, "default") == 0) {
    CG_Printf("You cannot rename a route 'default'. This name "
              "is protected for "
              "map entities trigger. \n");
    return;
  }

  const std::string tmp = oldName;
  const std::string tmp2 = newName;

  if (getRoutePositionByName(tmp2.c_str()) > -1) {
    CG_Printf("There already a route with name : %s \n", newName);
    return;
  }

  // Search in _routes
  const int z = getRoutePositionByName(tmp.c_str());
  if (z > -1) {
    // If the route is create by the mapper, that mean is
    // read-only.
    if (_routes[z].status == routeStatus::map) {
      CG_Printf("You can't rename this route. Mapper "
                "TJL are read-only. \n");
      return;
    }

    // Change name of the route.
    _routes[z].name = tmp2;
    CG_Printf("Route has been correctly rename to %s. \n", newName);
    return;
  }

  CG_Printf("No route with name : %s has been found to replace to %s. \n",
            oldName, newName);
  return;
}

float TrickjumpLines::normalizeSpeed(float max, float min, float speed) {
  return (speed - min) / (max - min);
}

void TrickjumpLines::computeHSV(float speed, vec3_t &hsv) {
  // Source from :
  // http://stackoverflow.com/questions/9507947/mapping-colors-to-an-interval
  const float hmax = 300; // This need to be hardcoded (0 to 300 mean
                          // red=0 to blue=300),

  // Simple x^1.35 * maxDegree to have a nice mapping function between
  // ups and color.
  hsv[0] = std::pow(speed, 1.35f) * hmax;
  hsv[1] = 1.0;
  hsv[2] = 1.0;

  // this is to revert color, blue = slow speed and red = high speed.
  hsv[0] = std::abs(hsv[0] - hmax);
}

void TrickjumpLines::hsv2rgb(vec3_t &hsv, vec3_t &rgb) {
  // Source from :
  // http://www.rapidtables.com/convert/color/hsv-to-rgb.htm

  // Compute simple parameter.
  const float c = hsv[1] * hsv[2];
  const int angleResult = hsv[0] / 60;
  const int moduloResult = angleResult % 2;
  const int absResult = std::abs(moduloResult - 1);
  const float x = c * (1 - absResult);
  const float m = hsv[2] - c;

  // Associate color to rgb depending on the angle of the HSV.
  float rPrime, gPrime, bPrime;
  if (angleResult == 0) {
    rPrime = c;
    gPrime = x;
    bPrime = 0.0f;
  } else if (angleResult == 1) {
    rPrime = x;
    gPrime = c;
    bPrime = 0.0f;
  } else if (angleResult == 2) {
    rPrime = 0.0f;
    gPrime = c;
    bPrime = x;
  } else if (angleResult == 3) {
    rPrime = 0.0f;
    gPrime = x;
    bPrime = c;
  } else if (angleResult == 4) {
    rPrime = x;
    gPrime = 0.0f;
    bPrime = c;
  } else // (angleResult == 5)
  {
    rPrime = c;
    gPrime = 0.0f;
    bPrime = x;
  }

  // Color are between 0-1, multiply by 255.0f to get color between
  // 0-255.
  rgb[0] = (rPrime + m) * 255.0f;
  rgb[1] = (gPrime + m) * 255.0f;
  rgb[2] = (bPrime + m) * 255.0f;
}

void TrickjumpLines::computeColorForNode(float max, float min, float speed,
                                         vec3_t &color) {
  // Normalize ups speed between 0-1
  const float norm = normalizeSpeed(max, min, speed);

  vec3_t hsvColor;
  // Compute associate HSV color with speed.
  computeHSV(norm, hsvColor);

  // Transform HSV color into RGB.
  hsv2rgb(hsvColor, color);

  return;
}

int TrickjumpLines::getRoutePositionByName(const char *name) {
  // Default has been pass as route name.
  const std::string tmp = name;
  if (tmp == "default") {
    return 0;
  }

  // Search in _routes.
  for (int z = 0; z < static_cast<int>(_routes.size()); ++z) {
    if (_routes[z].status == routeStatus::map) {
      if (_routes[z].name == tmp) {
        return z;
      }
    } else {
      if ((_routes[z].filename + std::string("_") + _routes[z].name) == tmp) {
        return z;
      }
    }
  }
  if (isDebug()) {
    CG_Printf("No route with name : %s has been found. \n", name);
  }

  return -1;
}

void TrickjumpLines::deleteRoute(const char *name) {
  if (name == nullptr) {
    CG_Printf("You need to pass a route name by argument. Use "
              "command "
              "/tjl_listroute to get name. \n");
    return;
  }

  const int z = getRoutePositionByName(name);
  if (z > -1) {
    if (_routes[z].status == routeStatus::map) {
      CG_Printf("You can't delete this route. Mapper "
                "TJL are read-only. \n");
      return;
    }
    _routes.erase(_routes.begin() + z);
    return;
  } else {
    CG_Printf("No route with this name. \n");
    return;
  }
}

void TrickjumpLines::toggleRoutes(bool state) {
  if (state) {
    CG_Printf("Trickjump line will be display. \n");
  } else {
    // CG_Printf("Trickjump line are now disable. This is only
    // toggle for the current session. Change cvar :
    // etj_tjlEnableLine for futur session. \n");
  }
  setEnableLine(state);
  return;
}

void TrickjumpLines::toggleMarker(bool state) {
  if (state) {
    CG_Printf("Trickjump marker will be display. \n");
  } else {
    // CG_Printf("Trickjump marker are now disable. This is only
    // toggle for the current session. Change cvar :
    // etj_tjlEnableMarker for futur session. \n");
  }
  setEnableMarker(state);
  return;
}
