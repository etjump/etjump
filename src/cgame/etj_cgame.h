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

#pragma once

#include <memory>
#include <vector>

namespace ETJump {
class ClientCommandsHandler;
class EntityEventsHandler;
class PlayerEventsHandler;
class AwaitedCommandHandler;
class CvarUpdateHandler;
class ClientRtvHandler;
class CustomCommandMenu;
class Timerun;

struct Handlers {
  std::shared_ptr<ClientCommandsHandler> serverCommands;
  std::shared_ptr<ClientCommandsHandler> consoleCommands;
  std::shared_ptr<EntityEventsHandler> entityEvents;
  std::shared_ptr<PlayerEventsHandler> playerEvents;
  std::unique_ptr<AwaitedCommandHandler> awaitedCommand;
  std::shared_ptr<CvarUpdateHandler> cvarUpdate;
  std::unique_ptr<ClientRtvHandler> rtv;
  std::unique_ptr<CustomCommandMenu> customCommandMenu;
  std::shared_ptr<Timerun> timerun;
};

class ClientAuthentication;
class OperatingSystem;
class SyscallExt;

struct Platform {
  std::unique_ptr<ClientAuthentication> authentication;
  std::unique_ptr<OperatingSystem> operatingSystem;
  std::unique_ptr<SyscallExt> syscallExt;
};

class DemoCompatibility;
class AutoDemoRecorder;

struct Demo {
  std::unique_ptr<DemoCompatibility> compatibility;
  std::unique_ptr<AutoDemoRecorder> autoDemoRecorder;
};

class EventLoop;
class CvarUnlocker;
class SavePos;
class ColorParser;
class TraceUtils;
class PmoveUtils;

struct Utils {
  std::unique_ptr<EventLoop> eventLoop;
  std::vector<std::unique_ptr<CvarUnlocker>> cvarUnlocker;
  std::unique_ptr<SavePos> savePos;
  std::unique_ptr<ColorParser> colorParser;
  std::unique_ptr<TraceUtils> trace;
  std::unique_ptr<PmoveUtils> pmove;
};

class ConsoleShader;
class IRenderable;

struct UI {
  std::unique_ptr<ConsoleShader> consoleShader;
  std::vector<std::unique_ptr<IRenderable>> renderables;
};

class LeavesRemapper;
class PlayerBBox;
class TrickjumpLines;

struct Visuals {
  std::unique_ptr<LeavesRemapper> leavesRemapper;
  std::unique_ptr<PlayerBBox> playerBBox;
  std::unique_ptr<TrickjumpLines> trickjumpLines;
};

class AccelColor;
class CHSDataHandler;
class TimerunView;

struct HUD {
  std::shared_ptr<AccelColor> accelColor;
  std::shared_ptr<CHSDataHandler> chsDataHandler;

  std::vector<std::unique_ptr<IRenderable>> renderables;
  std::unique_ptr<TimerunView> timerunView;
};

struct CGameContext {
  Handlers handlers;
  Platform platform;
  Demo demo;
  Utils utils;
  UI ui;
  Visuals visuals;
  HUD hud;
};
} // namespace ETJump
