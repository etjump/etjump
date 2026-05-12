# ETJump 3.5.0

## Added
* custom command menu - fully customizable, vsay-style menu to store any console commands/cvars in [#1801](https://github.com/etjump/etjump/pull/1801)
  * accessible via `openCustomCommandMenu` console command
  * commands are stored in `custom_commands.dat` by default, configured in TOML language
  * can store up to **40** commands, spread across 5 pages of the menu
* `func_static_client` entity - conditional brush entity with a unique state for each client [#1824](https://github.com/etjump/etjump/pull/1824)
* high resolution, anti-aliased CGaz 2 drawing (`etj_CGaz2HighRes`) [#1864](https://github.com/etjump/etjump/pull/1864)
  * `etj_CGaz2Thickness1/2` controls the thickness of the lines (`0.5 - 100.0`)
  * renders up to 1080px vertical resolution, before scaling to real screen resolution
* `listcheckpoints` and `comparecheckpoints` commands to list and compare checkpoint times from timeruns [#1829](https://github.com/etjump/etjump/pull/1829)
  * also accessible via admin commands `!list/comparecheckpoints`
* `etj_CGaz2Y` cvar to set Y position of CGaz 2 (`0.0 - 480.0`) [#1878](https://github.com/etjump/etjump/pull/1878)
* `etj_runTimerSize` to adjust timerun timer size (`0.0 - 10.0`) [#1814](https://github.com/etjump/etjump/pull/1814)
* 2 alternative mouse cursors (`etj_altCursor`) [#1812](https://github.com/etjump/etjump/pull/1812)
* `etj_cursorSize` to resize mouse cursor [#1812](https://github.com/etjump/etjump/pull/1812)
* `target_random` entity - fire events for a client based off random chance/number [#1771](https://github.com/etjump/etjump/pull/1771)
* `etj_CGaz1DrawMidLine` - draw a line between min/max angle in CGaz 1 [#1813](https://github.com/etjump/etjump/pull/1813)
  * `etj_CGaz1MidLineColor` adjusts the color
* link to ETJump documentation in in-game menus [#1886](https://github.com/etjump/etjump/pull/1886)
* `etj_onDemoPlaybackEnd` cvar to execute a command when demo playback ends [#1794](https://github.com/etjump/etjump/pull/1794)
* `printMapCustomizationInfo` console command to print custom mapscript/entity file hashes that the server uses [#1850](https://github.com/etjump/etjump/pull/1850)
  * works in demo playback, starting from demos recorded in ETJump 3.5.0
* `printDemoCompatInfo` console command to print active compatibility settings during demo playback [#1804](https://github.com/etjump/etjump/pull/1804)

## Changed
* increased limit for unique remappable shaders from **32** to **511**
* changelog improvements [#1885](https://github.com/etjump/etjump/pull/1885)
  * changelog entries are now categorized and sorted from most to least interesting/important
  * condensed changelog output to be easier to skim through
  * color highlighting in in-game changelog UI
  * slight visual tweaks to in-game UI
* vote text displays total(spectators) as vote counts, to help with ambiguity on vote results when spectator voting is enabled [#1793](https://github.com/etjump/etjump/pull/1793)
* improved smoothness of mouse cursor in menus [#1809](https://github.com/etjump/etjump/pull/1809)
  * overall mouse sensitivity in menus is slightly lower as a side effect
* mouse cursor now warps to center of the screen when UI is brought up [#1811](https://github.com/etjump/etjump/pull/1811)
  * chat UI is an exception, where cursor is warped to top left instead
* season states, timerun records and daily logs now use UTC time instead of local server time [#1862](https://github.com/etjump/etjump/pull/1862)
* server shutdowns due to an error are now logged to logfile [#1889](https://github.com/etjump/etjump/pull/1889)
* `g_mapScriptDir` can no longer be set to `maps` [#1880](https://github.com/etjump/etjump/pull/1880)
* `class` can be used to select single (**1**) or akimbo (**2**) pistols with 3rd argument [#1874](https://github.com/etjump/etjump/pull/1874)
* `callvote (dev)map` is no longer blocked on localhost [#1881](https://github.com/etjump/etjump/pull/1881)
* `etj_CGazY` now accepts decimal values [#1891](https://github.com/etjump/etjump/pull/1891)
* `ignore` no longer works on yourself [#1835](https://github.com/etjump/etjump/pull/1835)
* demo compatibility settings no longer automatically print to console on demo playback start [#1804](https://github.com/etjump/etjump/pull/1804)
* certain prints are now colored white instead of pale green [#1838](https://github.com/etjump/etjump/pull/1838)

## Fixed
* rank 1 times showed `+00:00.000` diff if a player had no record on a run [#1802](https://github.com/etjump/etjump/pull/1802)
* `etj_jumpSpeedsTextSize` was initialized from a wrong cvar value, requiring a value change before applying correctly [#1842](https://github.com/etjump/etjump/pull/1842)
* some votes weren't showing the keybinds for `vote yes/no` in the vote text [#1817](https://github.com/etjump/etjump/pull/1817)
* bad performance in score computation for timerun rankings, now faster by factor of hundred(s) depending on the server [#1831](https://github.com/etjump/etjump/pull/1831)
* cheat state preserved from previous map when trying to change it via vote in localhost (e.g. `devmap oasis` into `callvote randommap` would load next map with cheats enabled) [#1881](https://github.com/etjump/etjump/pull/1881)
* replay menu displayed demo names incorrectly if the filename ended in `^` [#1806](https://github.com/etjump/etjump/pull/1806)
* potential crash on older versions of ETe when issuing RCON commands in localhost [#1832](https://github.com/etjump/etjump/pull/1832)
* potential crash when loading a map after demo playback [#1844](https://github.com/etjump/etjump/pull/1844)
* `etj_smoothAngles` could sometimes apply on demo playback, breaking view angles [#1846](https://github.com/etjump/etjump/pull/1846)
* spectators were able to use `class` command [#1874](https://github.com/etjump/etjump/pull/1874)
* `etj_viewPlayerPortals` menu entry was missing a colon character [#1852](https://github.com/etjump/etjump/pull/1852)
* `etj_lagometerShader` menu tooltip listed an incorrect cvar name [#1875](https://github.com/etjump/etjump/pull/1875)

# ETJump 3.4.2

## Added
* `etj_jumpSpeedsTextSize` cvar to set the text size for jump speeds display (`0.1 - 10.0`) [#1787](https://github.com/etjump/etjump/pull/1787)
* warning print about loose `.menu` files if menu integrity checks fail without offending pk3 files [#1786](https://github.com/etjump/etjump/pull/1786)

## Changed
* quick connect menu now automatically refreshes on startup [#1785](https://github.com/etjump/etjump/pull/1785)
* tied records are now sorted from oldest to newest [#1775](https://github.com/etjump/etjump/pull/1775)
* clarified text about supported menu files in menu integrity check [#1770](https://github.com/etjump/etjump/pull/1770)

## Fixed
* client side coronas appeared incorrectly sized at certain scale values due to wraparound bug [#1783](https://github.com/etjump/etjump/pull/1783)
* diff time wasn't displaying for #1 record if your time was tied with it (common with 00:00.000 runs) [#1777](https://github.com/etjump/etjump/pull/1777)
* inactivity drop prints contained an excess newline [#1769](https://github.com/etjump/etjump/pull/1769)
* jump speeds were not working on demos recorded prior to ETJump 2.5.0 [#1776](https://github.com/etjump/etjump/pull/1776)
  * displayed speed is estimated and not fully accurate
* cvar size/scale parser failed to parse values starting with or containing excess whitespace [#1788](https://github.com/etjump/etjump/pull/1788)

# ETJump 3.4.1

## Added
* UI integrity checker - mod warns the user if they are running custom menu files and functionality cannot be guaranteed [#1759](https://github.com/etjump/etjump/pull/1759)

## Changed
* inactivity drop (`g_inactivity`) now saves and restores players' position when they join back to a team [#1748](https://github.com/etjump/etjump/pull/1748)
* `func_static` entities activated by dealing damage can no longer die under normal gameplay circumstances [#1744](https://github.com/etjump/etjump/pull/1744)

## Fixed
* some HUD elements were showing slightly incorrect data when spectating or playing back demos [#1753](https://github.com/etjump/etjump/pull/1753)
* changelog line splitting logic could get stuck in an infinite loop, causing game to hang when viewing some changelogs [#1745](https://github.com/etjump/etjump/pull/1745)
* projectiles were not removed on timerun start (broken since ETJump 3.3.1) [#1760](https://github.com/etjump/etjump/pull/1760)
* projectiles could be spawned by spawning them on a same frame as a timerun was started [#1760](https://github.com/etjump/etjump/pull/1760)
* `etj_autoLoad` bypassed checks for save slot timerun state, which allowed loading to positions that were not saved during a timerun [#1766](https://github.com/etjump/etjump/pull/1766)
* map progression tracked by `target/trigger_tracker` entities wasn't correctly saved and restored if player reconnected during same map [#1749](https://github.com/etjump/etjump/pull/1749)
* teleportation events could get interpolated if the player triggered multiple teleportation events during the same frame, causing visual bugs [#1758](https://github.com/etjump/etjump/pull/1758)
* sending chat messages wasn't clearing inactivity status from clients [#1747](https://github.com/etjump/etjump/pull/1747)
* inactive spawnpoints were visible on command map [#1743](https://github.com/etjump/etjump/pull/1743)
* deleting `target_spawn_relay` via mapscripting no longer crashes the server [#1752](https://github.com/etjump/etjump/pull/1752)
* `etj_hideMe` wasn't hiding players' portals correctly [#1756](https://github.com/etjump/etjump/pull/1756)
  * portals are still visible to other players who can use your portals
* flamethrower was producing dlights when trying to shoot underwater [#1746](https://github.com/etjump/etjump/pull/1746)
* flamethrower was playing idle sounds when underwater [#1746](https://github.com/etjump/etjump/pull/1746)

# ETJump 3.4.0

## Breaking
* `etj_viewPlayerPortals` now toggles drawing usable or all portals [#1706](https://github.com/etjump/etjump/pull/1706)
  * a portal is considered usable in the following scenarios:
    * the portal is yours
    * `portalteam` is set to 1 and the portal is yours or from a fireteam member
    * `portalteam` is set to 2 (anyone can use anyone's portals)

## Added
* support for client side predicted portalgun teleports [#1689](https://github.com/etjump/etjump/pull/1689)
  * disabled by default due to the potential for PVS-related visual bugs
  * `g_portalPredict` server cvar toggles predicted teleports (votable by clients)
    * voting can be disabled with `vote_allow_portalPredict`
* `portalpredict` worldspawn key to force predicted portal teleports for a map, overrides server cvar setting [#1689](https://github.com/etjump/etjump/pull/1689)
* "Quick connect" window in main menu to store up to 5 servers for easy connecting [#1701](https://github.com/etjump/etjump/pull/1701)
  * can be disabled with `etj_drawQuickConnectMenu` cvar
  * servers are stored in `quickconnect.dat` file
* short animation when portalgun portals are spawned [#1727](https://github.com/etjump/etjump/pull/1727)
* `target_ft_setrules` entity - set fireteam rules for activator's fireteam [#1625](https://github.com/etjump/etjump/pull/1625)
  * bypasses worldspawn keys that normally restrict setting rules for fireteam
* `noftsavelimit` and `noftteamjumpmode` worldspawn keys to disallow setting fireteam savelimit and teamjump mode [#1625](https://github.com/etjump/etjump/pull/1625)
* `fireteam countdown` command to perform a countdown in fireteam chat [#1703](https://github.com/etjump/etjump/pull/1703)
  * `fireteam countdown [seconds (1-10)]`, if `seconds` isn't given, value is taken from `etj_fireteamCountdownLength` cvar value
* `overbounce_players` worldspawn key to control overbounces performed on top of other clients [#1725](https://github.com/etjump/etjump/pull/1725)
* `demoQueue` command - automatically play back all demos from a directory [#1616](https://github.com/etjump/etjump/pull/1616) 
  * `etj_demoQueueDir` cvar sets sub-directory inside `demos` to play demos from, can be empty to play from `demos` directory directly
* `etj_logCenterPrint` cvar to log center prints to console [#1627](https://github.com/etjump/etjump/pull/1627)
  * consecutive, identical messages are not logged multiple times
  * several spammy prints (e.g. dynamite arming) and save messges are not logged
* `etj_spectatorInfoMaxClients` cvar to limit visible players in spectator info list (`-1` for no limit) [#1718](https://github.com/etjump/etjump/pull/1718)
  * a hint text will be displayed if the number of spectators exceeds the limit
* `etj_spectatorInfoDirection` cvar to toggle drawing spectator list bottom-up [#1718](https://github.com/etjump/etjump/pull/1718)
* `tracker_not_eq_any` and `tracker_not_eq_all` keys to `target/trigger_tracker` [#1614](https://github.com/etjump/etjump/pull/1614)
  * old `tracker_not_eq` is now deprecated and functionality is provided by `tracker_not_eq_any`
* `target_spawn_relay` entity - execute actions on player respawn [#1636](https://github.com/etjump/etjump/pull/1636)
* `etj_useExecQuiet` cvar to execute map/team-specific autoexec configs using `execq` command [1695](https://github.com/etjump/etjump/pull/1695)
  * `execq` executes a config "quietly" (no console print), supported by ET: Legacy and ETe
* `etj_hideFlamethrowerEffects` to hide the visual effects of flamethrower [#1726](https://github.com/etjump/etjump/pull/1726)
* `etj_onDemoPlaybackStart` cvar to execute commands on first frame of demo playback [#1617](https://github.com/etjump/etjump/pull/1617)
* `etj_noMenuFlashing` cvar to disable the lightning effect on the menu background shader [#1699](https://github.com/etjump/etjump/pull/1699)
* `etj_portalDebug` cvar to draw portal debug bounding (requires cheats) [#1689](https://github.com/etjump/etjump/pull/1689)
* `noise` key support for `weapon_portalgun` to override default item pickup sound [#1671](https://github.com/etjump/etjump/pull/1671)
* `dumpEntities` command to dump entities from the current map to a file [#1713](https://github.com/etjump/etjump/pull/1713)
* fixed `.arena` files for certain maps in the mod pk3 [#1690](https://github.com/etjump/etjump/pull/1690)

## Changed
* drawspeed2, CGaz, snaphud & strafe quality meter are now interpolated by default [#1674](https://github.com/etjump/etjump/pull/1674)
  * interpolation can be disabled per element with `etj_HUD_noLerp` cvar
* duplicate rank strings are now hidden from `records` output [#1603](https://github.com/etjump/etjump/pull/1603)
* `records` now shows total number of records for a run if all records don't fit on a single page [#1739](https://github.com/etjump/etjump/pull/1739)
* spectator list now draws active clients before inactive ones [#1718](https://github.com/etjump/etjump/pull/1718)
* static `dlight` entities are now fully client sided [#1668](https://github.com/etjump/etjump/pull/1668)
  * a dlight is considered static if it doesn't have `scriptname`, `targetname` or `spawnflags` set
  * client side dlights are limited to 1024 per map (server side limit is 16)
  * renderer is still limited to max 32 dlights in a rendered scene
* portalgun portals can no longer be overlapped in some scenarios if `portalteam` is enabled [#1705](https://github.com/etjump/etjump/pull/1705)
  * `portalteam 1` - fireteam members cannot overlap portals
  * `portalteam 2` - nobody can overlap portals
  * when joining a fireteam with `portalteam 1`, your portals are reset to prevent potential overlapping
* spectators can now use the fireteam general vsay menu [#1649](https://github.com/etjump/etjump/pull/1649)
* reworked timerun high ping interrupt to work based off command time delta (600ms) rather than ping [#1741](https://github.com/etjump/etjump/pull/1741)
* `g_dailyLogs` naming format was changed to use ISO 8061 date standard [#1680](https://github.com/etjump/etjump/pull/1680)
  * admin logs now have a `-admin` suffix rather than a prefix
* `page` parameter for `records` is now capped such that empty pages won't be printed [#1739](https://github.com/etjump/etjump/pull/1739)
* `viewpos` now prints viewangles for all axes [#1675](https://github.com/etjump/etjump/pull/1675)
* `setviewpos` can be used without giving angles as arguments, using current view angles [#1675](https://github.com/etjump/etjump/pull/1675)

## Removed
* location grid system - team/ft chats no longer display coordinates [#1651](https://github.com/etjump/etjump/pull/1651)
* `g_portalDebug` cvar [#1689](https://github.com/etjump/etjump/pull/1689)

## Fixed
* `weapon_portalgun` was missing a cursorhint for pickup [#1671](https://github.com/etjump/etjump/pull/1671)
* HWID generation on Linux was leaking a socket [#1652](https://github.com/etjump/etjump/pull/1652)
* `target_savereset` print contained a whitespace character at the beginning of the message [#1658](https://github.com/etjump/etjump/pull/1658)
* `etj_autoSpec` ignored jump and crouch when determining if a player was idle [#1662](https://github.com/etjump/etjump/pull/1662)
* improved performance of some HUD elements when spectating or playing back demos [#1663](https://github.com/etjump/etjump/pull/1663)
* `target_laser` could not deal damage to client 0 [#1669](https://github.com/etjump/etjump/pull/1669)
* `target_rumble` no longer crashes the server if re-triggered again before the previous rumble sequence had ended [#1670](https://github.com/etjump/etjump/pull/1670)
* client side coronas could be set to wrong color in some scenarios [#1673](https://github.com/etjump/etjump/pull/1673)
* scripted coronas were handled incorrectly on client rather than server [#1673](https://github.com/etjump/etjump/pull/1673)
* quitting during `etj_ad_stopDelay` period after a timerun was finished no longer crashes the game [#1380](https://github.com/etjump/etjump/pull/1380)
* chat replay could write a null JSON object in some scenarios [#1678](https://github.com/etjump/etjump/pull/1678)
* player's saves were sometimes not restored correctly when reconnecting to a server [#1681](https://github.com/etjump/etjump/pull/1681)
* potential crash when invalid vote argument was given [#1682](https://github.com/etjump/etjump/pull/1682)
* `players` command would crash the server with certain player names [#1685](https://github.com/etjump/etjump/pull/1685)
* "Unknown command uiChatMenuOpen" was printed to console when disconnecting via in-game menu [#1691](https://github.com/etjump/etjump/pull/1691)
* `portalteam 1` was not working and acted like `portalteam 2` [#1689](https://github.com/etjump/etjump/pull/1689)
* `func_portaltarget` created portals with incorrect size when `portalsize` key was used [#1689](https://github.com/etjump/etjump/pull/1689)
* `KEYS` menu button in `LAGOMETER` settings menu now works [#1702](https://github.com/etjump/etjump/pull/1702)
* centerprint when an objective was auto-returned due to timeout was missing for Allied objectives [#1704](https://github.com/etjump/etjump/pull/1704)
* portalgun trails and debug bboxes are now correctly colored green/yellow for other's portals [#1707](https://github.com/etjump/etjump/pull/1707)
* overbounce detector now works on top of other solid players [#1710](https://github.com/etjump/etjump/pull/1710)
* potential crash when hitting the 16th checkpoint of a timerun and checkpoint drawing was enabled [#1717](https://github.com/etjump/etjump/pull/1717)
* firing with `+attack2` didn't set the players firing state correctly, causing visual bugs with some weapons [#1719](https://github.com/etjump/etjump/pull/1719)
* scoreboard inactivity now correctly works if the server is using `sv_level/serverTimeReset` [#1731](https://github.com/etjump/etjump/pull/1731) [#1734](https://github.com/etjump/etjump/pull/1734)
* console no longer prints a warning when trying to vsay current class as a spectator [#1648](https://github.com/etjump/etjump/pull/1648)

# ETJump 3.3.4

## Changed
* reverted `etj_autoLoad` behavior change done in ETJump 3.3.1 [#1635](https://github.com/etjump/etjump/pull/1635)
* demo compatibility print for `sv_fps` now only prints when using a compatibility mode, instead of showing available detection sources [#1632](https://github.com/etjump/etjump/pull/1632)
* listbox & dropdown menus no longer change focused entry when dragging the scrollbar and hovering mouse over the entries [#1623](https://github.com/etjump/etjump/pull/1623)

## Fixed
* demo sub-directories in replays menu were not working on Windows [#1602](https://github.com/etjump/etjump/pull/1602)
* nametag interpolation now works for chat messages containing non-ASCII characters [#1606](https://github.com/etjump/etjump/pull/1606)
* chat replay stored interpolated names incorrectly if the name had an escape character [#1608](https://github.com/etjump/etjump/pull/1608)
* flamethrower was unable to set yourself on fire [#1607](https://github.com/etjump/etjump/pull/1607)
* flamechunks no longer visually interact with nonsolid players [#1607](https://github.com/etjump/etjump/pull/1607)
* spectator help text/follow text no longer disappears when chat window is opened [#1640](https://github.com/etjump/etjump/pull/1640)
* potential crash when interacting with flamechunks spawned by non-player entities (e.g. `props_flamethrower`) [#1607](https://github.com/etjump/etjump/pull/)
* flamethrower burn status no longer persists on respawn [#1637](https://github.com/etjump/etjump/pull/1637)
* reversed dropdown menus were not displaying background [#1626](https://github.com/etjump/etjump/pull/1626)
* potential buffer overflow with `stylestring` key on `dlight` entities [1622](https://github.com/etjump/etjump/pull/1622)

# ETJump 3.3.3

## Fixed
* crash on UI keyhandling on 2.60b and ET: Legacy due to out of bounds array access in engine [#1601](https://github.com/etjump/etjump/pull/1601)

# ETJump 3.3.2

## Breaking
* new cvar parsing system for cvars expecting size or scale values [#1590](https://github.com/etjump/etjump/pull/1590)
  * any cvar that expects a size or scale can now either take one or two values
    * if a single value is given, size/scale is set uniformly
    * if two values are given, size/scale is set independently on X/Y axes, respectively
* `etj_spectatorInfoSize` has been removed in favor of `etj_spectatorInfoScale` [#1590](https://github.com/etjump/etjump/pull/1590)
* `etj_crosshairScaleX/Y` are removed, same functionality can now be achieved with `cg_crosshairSize X Y` [#1590](https://github.com/etjump/etjump/pull/1590)

## Added
* support for per-client delays in `target_delay` (`spawnflag 1`) [#1591](https://github.com/etjump/etjump/pull/1591)
* `g_adminLog` now logs admin system and authentication related events [#1563](https://github.com/etjump/etjump/pull/1563)
  * admin commands using flags `b`, `C`, `A`, `k`, `m`, `P`, `R`, `s`, `T` and `c` are logged
  * authentication related events are logged (potential GUID/HWID spoofs, rejected connections due to a ban)
* alternating row colors for changelog UI [#1550](https://github.com/etjump/etjump/pull/1550)
* `toggleETJumpSettings` console command to open/close ETJump settings menu [#1562](https://github.com/etjump/etjump/pull/1562)
* `func_button` and `func_invisible_user` can show `wait` time as progress bar (`spawnflag 16`) [#1565](https://github.com/etjump/etjump/pull/1565)
  * slight inaccuracy with `func_button` - does not account for the travel time when the button returns to it's starting position
* `cursorhint` key support for `set/delete` script actions [#1584](https://github.com/etjump/etjump/pull/1584)
* `changeskin` script action for changing skin used on an entity [#1581](https://github.com/etjump/etjump/pull/1581)

## Changed
* brush entity limit increased from `255` to `511` [#1577](https://github.com/etjump/etjump/pull/1577)
* admin chats are now logged in `g_adminLog` instead of regular log file [#1563](https://github.com/etjump/etjump/pull/1563)
* `nojumpdelay` in `solstice` and `stonehalls2` now replicate a bug that was present in ETJump 2.3.0 - 2.5.0 [#1570](https://github.com/etjump/etjump/pull/1570)
* changelog UI now dims the screen for improved readability [#1546](https://github.com/etjump/etjump/pull/1546)
* slightly reduced window and font size of the changelog UI [#1546](https://github.com/etjump/etjump/pull/1546)
* improved error messages for some configstring overflows (commonly faced by mappers) [#1589](https://github.com/etjump/etjump/pull/1589)
* `entitylist` now shows the model number of brush entities [#1575](https://github.com/etjump/etjump/pull/1575)
* chat replay timestamp calculations are now performed on the server [#1597](https://github.com/etjump/etjump/pull/1597)
  * helps with system clock inaccuracies, which might cause a negative time display
* increased the number of files that can be read from a directory [#1571](https://github.com/etjump/etjump/pull/1571)

## Fixed
* `etj_autoSprint` broke if clients FPS dropped below 125 with `pmove_fixed 1` [#1556](https://github.com/etjump/etjump/pull/1556)
* players were producing water impacts when shot [#1544](https://github.com/etjump/etjump/pull/1544)
* `shooter_rocket` was calculating target deviation incorrectly [#1549](https://github.com/etjump/etjump/pull/1549)
* saving while dead could be used to get stuck in walls [#1552](https://github.com/etjump/etjump/pull/1552)
* players spawned at incorrect locations if another player was occupying a spawnpoint [#1555](https://github.com/etjump/etjump/pull/1555)
* chat name tagging broke on encoded chat messages if a player had `=` character in their name [#1561](https://github.com/etjump/etjump/pull/1561)
* flamethower now correctly ignores nonsolid players [#1598](https://github.com/etjump/etjump/pull/1598)
* portalgun was able to fire through solid players [#1599](https://github.com/etjump/etjump/pull/1599)
* `etj_drawSnapHUD 3` no longer draws borders at screen edges for zones which are off screen [#1557](https://github.com/etjump/etjump/pull/1557)
* `etj_hide` & `etj_hideDistance` was affecting `cg_drawCrosshairNames` draw range when fireteam collision was enabled [#1558](https://github.com/etjump/etjump/pull/1558)
* inconsistency in grouped popup counter formatting [#1585](https://github.com/etjump/etjump/pull/1585)
* timerun entity validation was not working correctly [#1592](https://github.com/etjump/etjump/pull/1592)
* mover health bar drawing was not respecting `cg_drawCrosshairNames` [#1558](https://github.com/etjump/etjump/pull/1558)
* minor memory leak in UI [#1567](https://github.com/etjump/etjump/pull/1567)
* console prints were duplicated in some scenarios on localhost [#1579](https://github.com/etjump/etjump/pull/1579)
* ETJump custom crosshairs were not drawing correctly in speaker editor [#1580](https://github.com/etjump/etjump/pull/1580)
* crosshair no longer disappears when zooming in speaker editor [#1580](https://github.com/etjump/etjump/pull/1580)
* `etj_fixedCompassShader` was missing from the settings menu [#1547](https://github.com/etjump/etjump/pull/1547)
* mounted MG42 overheat event wasn't correctly predicted [#1572](https://github.com/etjump/etjump/pull/1572)
* skins were not working on `misc_constructiblemarker` entities using a `model2` key [#1582](https://github.com/etjump/etjump/pull/1582)

# ETJump 3.3.1

## Added
* `etj_autoSprint` to flip `+sprint` button behavior - automatically sprint, hold down to run instead [#1519](https://github.com/etjump/etjump/pull/1519)
* experimental `etj_smoothAngles` for smoother viewangle updates at > 125FPS and `pmove_fixed 1` [#1495](https://github.com/etjump/etjump/pull/1495)
* changelogs are now included in the menus [#1532](https://github.com/etjump/etjump/pull/1532)
* `g_chatReplay` cvar to turn off chat replay on server [#1497](https://github.com/etjump/etjump/pull/1497)
  * chats are still logged when the feature is turned off, the messages are just not sent to clients
* `etj_snapHUDBorderThickness` to control `etj_drawSnapHUD 3` border thickness (`0.1 - 10.0` or `etj_snapHUDHeight * 2`)[#1494](https://github.com/etjump/etjump/pull/1494)
* engine extension support [#1531](https://github.com/etjump/etjump/pull/1531) 
* extended command backup/mask buffers on (ET: Legacy only) [#1535](https://github.com/etjump/etjump/pull/1535)
* `etj_viewlog` setting to the menus for ET: Legacy clients (works in ET: Legacy 2.83.0 and newer) [#1524](https://github.com/etjump/etjump/pull/1524)
* `etj_ad_stopInSpec` to automatically stop autodemo recording when switching to spectators [#1510](https://github.com/etjump/etjump/pull/1510) [#1542](https://github.com/etjump/etjump/pull/1542)
* `cursorhint` key support for `func_button` [#1507](https://github.com/etjump/etjump/pull/1507)
* `etj_highlight` bit **2** to flash window on chat mentions and incoming private messages on (ET: Legacy only) [#1531](https://github.com/etjump/etjump/pull/1531) 
* `sv_fps 125` compatible mapscript for `GreenJumps_f.bsp` in the mod pk3 [#1518](https://github.com/etjump/etjump/pull/1518)
* support for `author` key to `.arena` files (currently unused) [#1528](https://github.com/etjump/etjump/pull/1528)

## Changed
* chat replay messages are now timestamped with relative timestamps (10s ago, 1min ago, 1h ago etc.) [#1508](https://github.com/etjump/etjump/pull/1508)
* `g_chatReplayMaxMessageAge` now only applies to messages sent prior to players' session starting [#1521](https://github.com/etjump/etjump/pull/1521)
* `g_chatReplayMaxMessageAge` default value changed to **5** [#1521](https://github.com/etjump/etjump/pull/1521)
* `etj_hideMe` can no longer be enabled if fireteam player collision is enabled, and will automatically turn off [#1506](https://github.com/etjump/etjump/pull/1506)
* flamethrower no longer sets opposing team's players on fire [#1539](https://github.com/etjump/etjump/pull/1539)
* `changemodel` script action now works on `misc_gamemodel` entities [#1543](https://github.com/etjump/etjump/pull/1543)
* `ad_save` can no longer be used if the currently recorded demo isn't an autodemo temp demo [#1542](https://github.com/etjump/etjump/pull/1542)
* custom vote UI map list scrolling is now more responsive [#1534](https://github.com/etjump/etjump/pull/1534)
* custom vote UI map list now scrolls back to first entry when selecting a new list [#1534](https://github.com/etjump/etjump/pull/1534)
* adjusted timerun high ping interrupt to require sustained 100ms of lag before triggering [#1529](https://github.com/etjump/etjump/pull/1529)
* increased slick and NJD detector ranges to maximum possible map size [#1530](https://github.com/etjump/etjump/pull/1530)

## Fixed
* nonsolid players were pushing each other while riding on movers [#1526](https://github.com/etjump/etjump/pull/1526)
* fireteam `noghost` can no longer be enabled after starting a timerun which does not allow for `noghost` [#1520](https://github.com/etjump/etjump/pull/1520)
* backstab hint was showing for nonsolid players when holding a knife [#1492](https://github.com/etjump/etjump/pull/1492)
* knife no longer visually "hits" nonsolid players [#1492](https://github.com/etjump/etjump/pull/1492)
* corpses produced water impact particles when shot [#1501](https://github.com/etjump/etjump/pull/1501)
* in-game `Vote -> Map` was not working if `ui_netGameType` was not set to `2` [#1527](https://github.com/etjump/etjump/pull/1527)
* `etj_drawPlayerBBox` + `etj_hideFadeRange` was not correctly ignoring solid players [#1493](https://github.com/etjump/etjump/pull/1493)
* `etj_autoLoad` and `load` executed during a death sequence did not place the player initially at the save position [#1516](https://github.com/etjump/etjump/pull/1516)
* ammo packs were not disappearing when picked up by field ops, and gave incorrect amount of ammo [#1517](https://github.com/etjump/etjump/pull/1517)
* `etj_drawPlayerBBox` bit `4` was drawing bboxes for spectated players' fireteam instead of yours [#1538](https://github.com/etjump/etjump/pull/1538)
* client crash if server had a custom vote list with an empty `name` or `callvote_text` field [#1536](https://github.com/etjump/etjump/pull/1536)
* `target_init` and timerun start did not correctly clear ammo for removed weapons [#1515](https://github.com/etjump/etjump/pull/1515)
* `trigger_teleport_client` was not working correctly for spectators [#1511](https://github.com/etjump/etjump/pull/1511)
* bullet flesh impact sounds played at wrong location [#1500](https://github.com/etjump/etjump/pull/1500)
* bullet impacts sometimes failed to produce particle effects [#1503](https://github.com/etjump/etjump/pull/1503)
* reading a JSON file without a JSON object would crash the game [#1514](https://github.com/etjump/etjump/pull/1514)
* overbounce watcher broke when encountering an impossible overbounce (below Z coordinate, negative Z velocity) [#1496](https://github.com/etjump/etjump/pull/1496)
* flamethrower was doing inconsistent damage if `sv_fps` did not align to 50ms frametimes [#1541](https://github.com/etjump/etjump/pull/1541)
* `records` listing for multiple seasons was missing a newline between the seasons [#1525](https://github.com/etjump/etjump/pull/1525)
* `TAB` and `UP/DOWNARROW` keys closed dropdown menus unexpectedly [#1533](https://github.com/etjump/etjump/pull/1533)

# ETJump 3.3.0

## Breaking
* `cg_recording_statusline` cvar has been removed [#1475](https://github.com/etjump/etjump/pull/1475)

## Added
* ability to toggle player collision between fireteam members [#1333](https://github.com/etjump/etjump/pull/1333)
  * `fireteam rules noghost <1|on>/<0/off>` to enable/disable, also available in fireteam rule menu
* `noftnoghost` worldspawn key to disallow toggling fireteam collision in a map [#1333](https://github.com/etjump/etjump/pull/1333)
* `spawnflag 256` to `target/trigger_starttimer` to allow toggling fireteam collission during a timerun [#1333](https://github.com/etjump/etjump/pull/1333)
* chat replay system - server replays 10 latest global chat messages to clients after connecting/map change/`vid_restart` [#1335](https://github.com/etjump/etjump/pull/1335)
  * chats are stored on server in `chatreplay.json`
  * timestamps or chat flags are not preserved from original messages
* `g_chatReplayMaxMessageAge` cvar to specify maximum age of a chat message included in chat replay (**0** = no limit) [#1488](https://github.com/etjump/etjump/pull/1488)
* `etj_chatReplay` cvar to turn off chat replay on client side [#1377](https://github.com/etjump/etjump/pull/1377)
  * any chat messages you send are still included in other players' chat replays
* `etj_autoSpec` cvar automatically follow next client when idling in free spec [#1382](https://github.com/etjump/etjump/pull/1382)
  * `etj_autoSpecDelay` - time in milliseconds to wait before automatically following next client
* `savepos/loadpos` system - alternative to regular `save/load`, preserving speed and timerun state [#1456](https://github.com/etjump/etjump/pull/1456)
  * `savepos` can be used at any time (including demo playback), `loadpos` requires cheats
  * timerun state saving/loading only works on demos from ETJump 3.3.0 and newer
* dropdown menus in UI for more convenient multi-selection menu entries [#1406](https://github.com/etjump/etjump/pull/1406)
* color picker for UI, accessible via `Color picker...` entry on dropdown menus for cvars adjusting colors [#1422](https://github.com/etjump/etjump/pull/1422)
* custom votes in vote UI - view and vote custom vote lists from the in-game vote menu [#1447](https://github.com/etjump/etjump/pull/1447)
* adminchat channel for chat, available for players with adminflag `S` [#1415](https://github.com/etjump/etjump/pull/1415)
  * can be disabled on the server with `g_adminChat`
  * `say_admin`, `ma` and `enc_say_admin` will send a message to adminchat
  * `adminChat` console command opens the chat window with adminchat selected
* `trigger_teleport_client` entity for client side predicted teleports [#1332](https://github.com/etjump/etjump/pull/1332)
* `portalsize` key for `func_portaltarget` to allow scaling the size of the portal fired onto it (up to `512u`) [#1324](https://github.com/etjump/etjump/pull/1324)
* support for short flag names in the new command system (e.g. `--help` = `-h`, see help of each command for details) [#1338](https://github.com/etjump/etjump/pull/1338)
* `etj_drawPlayerBBox` to draw bounding boxes of players [#1372](https://github.com/etjump/etjump/pull/1372)
  * `etj_playerBBoxBottomOnly` - draw only bottom of bbox instead of full box
  * `etj_playerBBoxShader` - shader to use for drawing
  * `etj_playerBBoxColorSelf/Other/Fireteam` - sets the color of the bbox (only works for shaders using `rgbGen vertex`)
* `g_allowSpeclock` to let servers disable speclocking [#1358](https://github.com/etjump/etjump/pull/1358)
* several new spawnflags for `target_ftrelay`, mostly matching `target_relay` [#1364](https://github.com/etjump/etjump/pull/1364)
  * `spawnflag 4` fires all targets instead of one random target
  * `spawnflag 32` only fires for other fireteam members
* `spawnflag 32` for teleport entities to disable any Z-offsetting of position [#1360](https://github.com/etjump/etjump/pull/1360)
* `spawnflag 4096` for `trigger_multiple` to disable activation while noclipping [#1368](https://github.com/etjump/etjump/pull/1368)
* `etj_lagometerAlpha` to control transparency of lagometer background [#1325](https://github.com/etjump/etjump/pull/1325)
* `etj_lagometerShader` to toggle displaying the background image or solid color [#1325](https://github.com/etjump/etjump/pull/1325)
* `delete` script action for deleting entities by their entity key/value pairs [#1481](https://github.com/etjump/etjump/pull/1481)
* client/server snapshot rate display in lagometer [#1331](https://github.com/etjump/etjump/pull/1331)
* support for `private` keyword to `playsound` script actions to allow playing sound only to activator [#1314](https://github.com/etjump/etjump/pull/1314)
* console info print at the start of demo playback, displaying information about the demo file [#1470](https://github.com/etjump/etjump/pull/1470)
* "cached" menu sliders that only update the cvar value on key up [#1466](https://github.com/etjump/etjump/pull/1466)
  * used by `etj_noClipScale` and `etj_menuSensitivity` as of now
* cvars for controlling demo recording status line [#1475](https://github.com/etjump/etjump/pull/1475)
  * `etj_drawRecordingStatus` - toggle demo recording status on/off
  * `etj_recordingStatusX/Y` - X/Y position
* `etj_drawSnapHUD 3` cvar to draw snaphud with borders only instead of solid colored blocks [#1430](https://github.com/etjump/etjump/pull/1430)
* fireteam menu option to toggle teamjump mode [#1364](https://github.com/etjump/etjump/pull/1364)
* indicator to fireteam overlay to show if teamjump mode is active [#1364](https://github.com/etjump/etjump/pull/1364)
* scrolling briefing text for `Vote -> Map -> Details` & `Host Game` menus if text does not fit on screen [#1442](https://github.com/etjump/etjump/pull/1442)

## Changed
* `Vote -> Map` menu now lists all maps on the server, even for servers with lots of pk3s [#1431](https://github.com/etjump/etjump/pull/1431)
  * the list is also now sorted alphabetically
* timerun timer displays white for tied records and first records (no previous record) [#1367](https://github.com/etjump/etjump/pull/1367)
* projectiles and bullets now collide with solid players [#1333](https://github.com/etjump/etjump/pull/1333)
* custom vote list names can no longer be just color codes [#1369](https://github.com/etjump/etjump/pull/1369)
* custom vote lists now force mapnames to lowercase [#1369](https://github.com/etjump/etjump/pull/1369)
* lean angles are now interpolated on spec/demo playback to smooth out viewangle transitions [#1337](https://github.com/etjump/etjump/pull/1337)
* menu entries with `Custom` value now display the actual cvar value [#1416](https://github.com/etjump/etjump/pull/1416)
* `shooter_rocket/grenade/mortar` no longer spawn entities if entity limit is close to full [#1467](https://github.com/etjump/etjump/pull/1467)
* changes to the collectible token system [#1334](https://github.com/etjump/etjump/pull/1334)
  * maximum tokens per difficulty increased to **32**
  * tokens are now placed at player's feet rather than origin
  * token bbox matches the visual size more accurately
  * tokens can no longer be collected while noclipping
* lagometer shows snapshot delta as ping on demo playback [#1331](https://github.com/etjump/etjump/pull/1331)
* `target_ftrelay` no longer fires for spectators [#1364](https://github.com/etjump/etjump/pull/1364)
* `!userinfo` output displays players' IP address if they are connected [#1417](https://github.com/etjump/etjump/pull/1417)
* menu options that are incompatible with players client are now hidden [#1413](https://github.com/etjump/etjump/pull/1413)
* improved runtimer accuracy when server is running at `sv_fps 125` [#1468](https://github.com/etjump/etjump/pull/1468)
* menu sliders no longer send cvar updates every frame if not moving the mouse [#1455](https://github.com/etjump/etjump/pull/1455)
* dragging a scrollbar in listbox menu no longer stops the scroll if cursor moves outside the window [#1424](https://github.com/etjump/etjump/pull/1424)
* `set` script action no longer works for setting `classname` outside of `spawn` script blocks [#1480](https://github.com/etjump/etjump/pull/1480)
* `listinfo/customvotes` output indicates if server has no lists defined [#1463](https://github.com/etjump/etjump/pull/1463)
* `!rename` now fails if the new name is too long [#1485](https://github.com/etjump/etjump/pull/1485)
* improved fireteam prints related to teamjump mode [#1364](https://github.com/etjump/etjump/pull/1364)
* improved logging for tokens, motd and custom votes [#1463](https://github.com/etjump/etjump/pull/1463)
* various errors from `!tokens` command are now handled gracefully instead of crashing the server [#1463](https://github.com/etjump/etjump/pull/1463)
* `cg_fov` can be set outside of **90 - 160** range in demo playback without `developer 1` [#1489](https://github.com/etjump/etjump/pull/1489)

## Removed
* `g_debugTimeruns` cvar, enabling cheats provides everything that this did [#1469](https://github.com/etjump/etjump/pull/1469)
* removed all cvars and commands related to gametypes (apart from `g_gametype`) [#1341](https://github.com/etjump/etjump/pull/1341)

## Fixed
* server side framerate dependencies - the mod can now run at `sv_fps 125` [#1327](https://github.com/etjump/etjump/pull/1327) [#1330](https://github.com/etjump/etjump/pull/1330) [#1407](https://github.com/etjump/etjump/pull/1407)
  * `wait` key in mapscripts simulates `sv_fps 20` timings
  * disguise stealing speed normalized to `sv_fps 20`
  * flamethrower firing range normalized to `sv_fps 20`
  * flamethrower damage normalized to `sv_fps 20`
  * player pushing when players are stuck in each other is normalized to `sv_fps 20`
  * projectiles entering skyboxes no longer get stuck in skyboxes due to higher trace frequency
* systemcalls were working unreliably on 64-bit clients [#1396](https://github.com/etjump/etjump/pull/1396)
* `etj_autoPortalBinds` triggered `weapalt -> +attack2` rebinds when spectating someone holding a portalgun [#1363](https://github.com/etjump/etjump/pull/1363)
* server side coronas could not be turned off [#1352](https://github.com/etjump/etjump/pull/1352)
* `!ban` did not accept **0** as time for permanent ban [#1418](https://github.com/etjump/etjump/pull/1418)
* fireteam health was not displaying correctly for members who were on a different team [#1320](https://github.com/etjump/etjump/pull/1320)
* vote start/end was resetting event handler, causing unexpected side effects when a vote started/ended [#1346](https://github.com/etjump/etjump/pull/1346)
* `!spectate` did not interrupt timeruns which were supposed to reset on team switch [#1375](https://github.com/etjump/etjump/pull/1375)
* `records <runname>` was returning additional partial matches when an exact run name was given [#1371](https://github.com/etjump/etjump/pull/1371)
* `!add-customvote` would delete existing customvote file if the file had a syntax error [#1443](https://github.com/etjump/etjump/pull/1443)
* timeruns no longer interrupt due to too low fps with `com_maxfps 0` [#1405](https://github.com/etjump/etjump/pull/1405)
* negative values for `give health` were not killing players correctly [#1328](https://github.com/etjump/etjump/pull/1328)
* collision with `func_fakebrush` using `CONTENTS_PLAYECLIP` was not working correctly [#1429](https://github.com/etjump/etjump/pull/1429)
* viewangles no longer stutter when riding on movers while prone or using a mortar set [#1353](https://github.com/etjump/etjump/pull/1353)
* fireteam vsays were not working when a member was selected with `selectbuddy` [#1362](https://github.com/etjump/etjump/pull/1362)
* fireteam chat was possible without being on a fireteam [#1362](https://github.com/etjump/etjump/pull/1362)
* portalgun portals were drawing mirrored on 2.60b clients [#1388](https://github.com/etjump/etjump/pull/1388)
* `spawnflag 2` was not working on some weapon entities [#1365](https://github.com/etjump/etjump/pull/1365)
* `+attack2` was not working on mounted MG42s, now fires normally like with other weapons [#1376](https://github.com/etjump/etjump/pull/1376)
* `!listbans` output was affected by the color of "banned by" entry [#1419](https://github.com/etjump/etjump/pull/1419)
* `!rename` ignored spaces for the new name [#1485](https://github.com/etjump/etjump/pull/1485)
* unrelated fireteam/vote messages could be visible simultaneously [#1383](https://github.com/etjump/etjump/pull/1383)
* demo list sorting was not case-insensitive [#1437](https://github.com/etjump/etjump/pull/1437)
* player shaders had incorrect sorting, drawing them over muzzleflashes [#1361](https://github.com/etjump/etjump/pull/1361)
* `!tokens move` was not placing tokens at correct height [#1463](https://github.com/etjump/etjump/pull/1463)
* water impact markers were not working correctly in some scenarios [#1355](https://github.com/etjump/etjump/pull/1355)
* `Write Config` button in settings menu was not focusing on text field upon opening [#1425](https://github.com/etjump/etjump/pull/1425)
* `Vote -> Map -> Details` menu was not updating when new map was selected on vote list [#1442](https://github.com/etjump/etjump/pull/1442)
* `Vote -> Map -> Details` menu displayed levelshot at wrong aspect ratio [#1442](https://github.com/etjump/etjump/pull/1442)
* excess overheat events were triggering for mounted MG42s [#1376](https://github.com/etjump/etjump/pull/1376)
* chat highlighting was not working for server chat messages [#1381](https://github.com/etjump/etjump/pull/1381)
* several crashes related to JSON parsing [#1463](https://github.com/etjump/etjump/pull/1463)
* menu loading fallback was not working if custom menufile was not found [#1446](https://github.com/etjump/etjump/pull/1446)
* potential crash on host game menu when over 500 maps were installed [#1343](https://github.com/etjump/etjump/pull/1343)
* missing newlines on some `records` prints [#1378](https://github.com/etjump/etjump/pull/1378)
* various backwards compatibility issues with old demos [1472](https://github.com/etjump/etjump/pull/1472)

# ETJump 3.2.2

## Added
* warning message in console if a gamemodel cannot be loaded [#1313](https://github.com/etjump/etjump/pull/1313)

## Fixed
* CGaz 2 velocity direction lines were flickering on spec/demo playback when playing at > 125FPS [#1307](https://github.com/etjump/etjump/pull/1307)
* `etj_CGaz1DrawSnap` was flickering on GCC builds due to fast math optimizations [#1316](https://github.com/etjump/etjump/pull/1316)
* noclipping inside `nonoclip` zones was possible if the player was crouched or prone, and had no room to stand up [#1323](https://github.com/etjump/etjump/pull/1323)
* `etj_autoPortalBinds` did not reset bindings when switching to spectators [#1318](https://github.com/etjump/etjump/pull/1318)
* crash with model tag loader on ET 2.60b [#1317](https://github.com/etjump/etjump/pull/1317)
* demo playback from the root of `demos` directory was not working on ET: Legacy clients on Linux [#1310](https://github.com/etjump/etjump/pull/1310)
* coronas were incorrectly culled from view with `etj_realFov 1` and wide aspect ratios [#1311](https://github.com/etjump/etjump/pull/1311)

# ETJump 3.2.1

## Added
* `etj_CGaz2WishDirUniformLength` cvar to draw CGaz 2 wishdir lines at consistent length regardless of wishdir direction [#1303](https://github.com/etjump/etjump/pull/1303)
* `etj_CGaz2WishDirFixedSpeed` cvar to scale wishdir line length by speed (similar to `etj_CGaz2FixedSpeed`) [#1303](https://github.com/etjump/etjump/pull/1303)

## Changed
* `etj_CGaz2FixedSpeed` is no longer capped to 1200ups [#1303](https://github.com/etjump/etjump/pull/1303)

## Fixed
* server crash when Auto RTV was called [#1293](https://github.com/etjump/etjump/pull/1293)
* server crash with `callvote rtv <list>` if the list had less valid maps that `g_rtvMapCount` [#1294](https://github.com/etjump/etjump/pull/1294)
* speedmeter sometimes showed 0ups when landing on spec/demo playback [#1296](https://github.com/etjump/etjump/pull/1296)
* advanced accel drawing was not turning back on automatically after joining a team from spectators [#1296](https://github.com/etjump/etjump/pull/1296)
* accel speed flickered on spec/demo playback [#1296](https://github.com/etjump/etjump/pull/1296)
* footstep cycle could get desynced in some scenarios [#1297](https://github.com/etjump/etjump/pull/1297)
* `etj_CGaz1DrawSnapZone` was flickering in some scenarios [#1300](https://github.com/etjump/etjump/pull/1300) [#1301](https://github.com/etjump/etjump/pull/1301)
* `etj_CGaz1DrawSnapZone` did not respect `etj_CGazTrueness` [#1302](https://github.com/etjump/etjump/pull/1302)
* typo in `com_hunkmegs` menu tooltip [#1298](https://github.com/etjump/etjump/pull/1298)

# ETJump 3.2.0

## Added
* native support for Apple Silicon on macOS (universal binaries) [#1208](https://github.com/etjump/etjump/pull/1208)
* `etj_drawAccel` to display acceleration on x/y vectors [#1183](https://github.com/etjump/etjump/pull/1183) [#1258](https://github.com/etjump/etjump/pull/1258)
* value **2** for`etj_accel/speedColorUsesAccel` to displaying "advanced" accel color [#1183](https://github.com/etjump/etjump/pull/1183)
  * not available while spectating/on demo playback due to interpolation inaccuracy
* `func_portaltarget` entity - portals fired on this entity area always centered on the face that they hit [#1188](https://github.com/etjump/etjump/pull/1188)
* `etj_CGaz1DrawSnapZone` cvar to draw current snap zone on CGaz 1 [#1175](https://github.com/etjump/etjump/pull/1175) [#1268](https://github.com/etjump/etjump/pull/1268)
  * uses `etj_CGazFov`, will not align to snaphud if using different fov on both
* `callvote rtv <list>` to call RTV with maps from a custom vote list [#1215](https://github.com/etjump/etjump/pull/1215) [#1224](https://github.com/etjump/etjump/pull/1224)
* `wait` key for `trigger_push` to limit time between activations (default **100ms**, valid range **0 - 65535**) [#1217](https://github.com/etjump/etjump/pull/1217)
* `etj_CGaz2NoVelocityDir 2` to hide velocity direction only while under wishspeed [#1257](https://github.com/etjump/etjump/pull/1257)
* `etj_snapHUDActiveIsPrimary` cvar - always use `etj_snapHUDColor1` for active zone and flip colors when changing zones [#1182](https://github.com/etjump/etjump/pull/1182)
* `spawnflag 8` for `func_invisible_user` to pass activator to mapscript for per-client actions [#1209](https://github.com/etjump/etjump/pull/1209)
* `usetarget` mapscript action - similar to `alertentity` but passes activator to the targeted entity [#1209](https://github.com/etjump/etjump/pull/1209)
* mapscripting support for progression trackers [#1207](https://github.com/etjump/etjump/pull/1207)
* bit support for tracker entities [#1205](https://github.com/etjump/etjump/pull/1205)
* center print when trying to noclip in a `nonoclip` area [#1231](https://github.com/etjump/etjump/pull/1231)
* support for `weapon_grenadelauncher` and `weapon_grenadepineapple` entities (axis/allied hand grenade) [#1195](https://github.com/etjump/etjump/pull/1195) [#1246](https://github.com/etjump/etjump/pull/1246)
* `func_missilepad` entity - explodes hand & rifle grenades on impact [#881](https://github.com/etjump/etjump/pull/881)
* `etj_muzzleFlash` to control muzzleflash drawing [#1265](https://github.com/etjump/etjump/pull/1265)
* icons for replay menu for directories and up/home options [#1278](https://github.com/etjump/etjump/pull/1278)
* background toggler in ETJump settings menu [#1284](https://github.com/etjump/etjump/pull/1284)
* `writeconfig` button in ETJump settings menu [#1284](https://github.com/etjump/etjump/pull/1284)
* support for `classname_nospawn` key with `set` script action to change entity's `classname` without calling it's spawn method [#1228](https://github.com/etjump/etjump/pull/1228)
* support for modifying `trigger_objective_info` via `set` mapscript action [#1239](https://github.com/etjump/etjump/pull/1239)

## Changed
* overhauled settings menu with tab-based layout + submenus [#1284](https://github.com/etjump/etjump/pull/1284)
* entity health bar now has gradient color and a small border [#1185](https://github.com/etjump/etjump/pull/1185)
* static coronas are fully client sided, freeing up entity slots [#1201](https://github.com/etjump/etjump/pull/1201)
* `etj_autoPortalBinds` default is now **1** [#1259](https://github.com/etjump/etjump/pull/1259)
* reworked `cg_autoswitch` cvar to allow more granularity on autoswitch logic [#1266](https://github.com/etjump/etjump/pull/1266) [#1269](https://github.com/etjump/etjump/pull/1269)
* improved callvote texts on some votes [#1214](https://github.com/etjump/etjump/pull/1214)
  * `callvote map` -> `Change map to <mapname>`
  * `callvote devmap` -> `Change map to <mapname> (cheats enabled)`
  * `callvote randommap <maplist>` -> `Random map from <listname>`
* `goto/call/iwant` copies stance from the target/caller [#1223](https://github.com/etjump/etjump/pull/1223)
* `entitylist` displays `scriptname` field of entites and the amount of brush entities in a map [#1235](https://github.com/etjump/etjump/pull/1235) [#1275](https://github.com/etjump/etjump/pull/1275)
* `!rename` no longer reduces name change limit (unless using it on yourself) [#1219](https://github.com/etjump/etjump/pull/1219)
* re-vote cooldown reduced from **2s** to **1s** [#1241](https://github.com/etjump/etjump/pull/1241)
* speaker editor visuals match rest of the UI style better [#1203](https://github.com/etjump/etjump/pull/1203)
* `map/devmap` commands force mapname to lowercase for consistency with callvote [#1254](https://github.com/etjump/etjump/pull/1254)

## Fixed
* client side (`com_maxFPS`) framerate dependencies [#1229](https://github.com/etjump/etjump/pull/1229)
  * pmove related (only relevant with `pmove_fixed 0`):
    * weapon recoil normalized to ~125FPS (slightly deviates on >333FPS)
    * stamina recharge is consistent on any FPS
    * weapon overheating normalized to 125FPS
    * viewbob cycle & footsteps normalized to 125FPS
    * corpse movement normalized to 125FPS
  * visual fixes:
    * screenshakes from explosions normalized to 125FPs
    * kickangles (viewkick from shooting) normalized to 333FPS
    * particle effect rotation (`cg_wolfparticles`) normalized to 125FPS
* crash when timerun started or player activated `target_init` with `spawnflag 128` while cooking a grenade and selfkilling afterwards [#1196](https://github.com/etjump/etjump/pull/1196)
* `setoffset` could be used to go into/through `nonoclip` areas [#1192](https://github.com/etjump/etjump/pull/1192)
* `etj_autoLoad` was not working correctly when switching between Axis and Allies [#1252](https://github.com/etjump/etjump/pull/1252)
* noclip was not deactivating for spectators when entering a `nonoclip` area [#1231](https://github.com/etjump/etjump/pull/1231)
* `tracker_not_eq` ignored failed checks from other tracker keys [#1204](https://github.com/etjump/etjump/pull/1204)
* chat mentions broke team/fireteam chat color [#1200](https://github.com/etjump/etjump/pull/1200)
* user database could sometimes write garbage data when storing nicknames [#1206](https://github.com/etjump/etjump/pull/1206)
* OB detector ignored `nooverbounce` on demo playback [#1243](https://github.com/etjump/etjump/pull/1243)
* timeruns with `spawnflag 16` blocked other save slots from being loaded when the slot was not a timerun save [#1289](https://github.com/etjump/etjump/pull/1289)
* changing `sv_fps` did not work without reloading a map [#1260](https://github.com/etjump/etjump/pull/1260)
* `misc_gamemodel` entities using weapon models were drawing with a semi-transparent shader [#1282](https://github.com/etjump/etjump/pull/1281)
* `set` mapscript action could crash the game when used to change `classname` key [#1228](https://github.com/etjump/etjump/pull/1228)
* `!rename` could be used on same level/higher admins [#1219](https://github.com/etjump/etjump/pull/1219)
* `load` could be used to get into prone stance with mortar set [#1221](https://github.com/etjump/etjump/pull/1221)
* `target_remove_portals` only played sound for client 0 [#1286](https://github.com/etjump/etjump/pull/1286)
* `target_init` with `spawnflags 4/128` was not respecting `spawnflags 8` [#1291](https://github.com/etjump/etjump/pull/1291)
* noclipping was possible while dead if activated on the same frame as the player died [#1232](https://github.com/etjump/etjump/pull/1232)
* dlights were not correctly updated if `set` was used to change an existing entity into a dlight [#1245](https://github.com/etjump/etjump/pull/1245)
* noclipping spectators appeared as Axis soldiers in 3rd person and freecam [#1186](https://github.com/etjump/etjump/pull/1186)
* `HOME/END` keys broke input for numeric fields in menus [#1267](https://github.com/etjump/etjump/pull/1267)
* speaker editor strings displayed garbage data [#1203](https://github.com/etjump/etjump/pull/1203)
* speaker editor was not correctly working on widescreen resolutions [#1203](https://github.com/etjump/etjump/pull/1203)
* multiline tooltips sometimes rendered at incorrect height [#1290](https://github.com/etjump/etjump/pull/1290)
* UI memory pool increased to make the mod work on Linux 2.60b [#1191](https://github.com/etjump/etjump/pull/1191)
* class and skill icon mouseover areas were misaligned in limbo menu on widescreen resolutions [#1225](https://github.com/etjump/etjump/pull/1225)

# ETJump 3.1.1

## Changed
* timeruns can no longer be started if `setoffset` was used, without selfkilling first [#1178](https://github.com/etjump/etjump/pull/1178)
* switching to spectators now always interrupts a timerun, even when team switches are allowed - only Axis <-> Allies switches are valid [#1168](https://github.com/etjump/etjump/pull/1168)
* changing auto RTV settings via vote now has better feedback [#1164](https://github.com/etjump/etjump/pull/1164)
  * successful vote tells whether it was turned on, or the interval was adjusted
* 2 minute warning no longer prints for auto RTV if it gets adjusted via vote such that there's less than 2 minutes until next vote [#1174](https://github.com/etjump/etjump/pull/1174)
* `random` parameter for `(global)accum` now requires a non-zero value [#1173](https://github.com/etjump/etjump/pull/1173)
* `rate` parameter on `playanim` script action was clamped to **1 - 999** [#1173](https://github.com/etjump/etjump/pull/1173)

## Fixed
* multiple `trigger_multiple` entities could not be activated simultaneously [#1163](https://github.com/etjump/etjump/pull/1163)
* `surfaceparm nodamage` was playing doubled footsteps [#1166](https://github.com/etjump/etjump/pull/1166)
* OB detector Y position was too high compared to old versions [#1162](https://github.com/etjump/etjump/pull/1162)
* RTV menu did not draw if a player connected after/during the first RTV vote of the map [#1167](https://github.com/etjump/etjump/pull/1167)
* `vote_allow_rtv 0` was blocking auto RTV from getting called [#1164](https://github.com/etjump/etjump/pull/1164)
* potential crash in client rtv menu when `g_rtvMapCount` was set to < 9 [#1180](https://github.com/etjump/etjump/pull/1180)
* `etj_fixedCushionSteps` footstep sound prediction was broken [#1166](https://github.com/etjump/etjump/pull/1166)
* auto RTV could be voted off when it was already off [#1164](https://github.com/etjump/etjump/pull/1164)
* `g_oss` incorrectly indicated that macOS arm64 was supported [#1177](https://github.com/etjump/etjump/pull/1177)
* `(global)accum` indices 8 and 9 were working unreliably [#1170](https://github.com/etjump/etjump/pull/1170)

# ETJump 3.1.0

## Added
* Rock The Vote system - vote the next map from a random selection of maps [#1135](https://github.com/etjump/etjump/pull/1135)
  * `callvote rtv` or `!rtv` admin command to call the vote
  * automatic RTV mode - server will call RTV every N minutes, can be voted on/off (`g_autoRtv`)
  * **2 - 9** maps can be in a vote, controlled by `g_rtvMaxMaps`
* no-noclip area support with `surfaceparm donotenterlarge` [#1157](https://github.com/etjump/etjump/pull/1157)
  * `etj_drawNoclipIndicator` controls drawing of noclip area indicator
* `etj_checkpointsPopup` to display checkpoint times briefly on screen after a checkpoint is hit [#1139](https://github.com/etjump/etjump/pull/1139)
* custom vote editing via admin commands [#1152](https://github.com/etjump/etjump/pull/1152)
* `etj_autoPortalBinds` cvar to automatically bind `weapalt` to `+attack2` and back when switching to/from portalgun [#1125](https://github.com/etjump/etjump/pull/1125)
* `etjump_game_manager` entity - entry point for mapscripting in maps that don't support mapscripting otherwise [#1123](https://github.com/etjump/etjump/pull/1123)
  * provides `etjump_manager` script block for maps with no `script_multiplayer` or an entity with `scriptname` set
* `wm_announce_private` script action for doing per-client announces via mapscript [#1083](https://github.com/etjump/etjump/pull/1083)
* support for printing activators name with `%s` in `wm_announce/wm_announce_private` [#1085](https://github.com/etjump/etjump/pull/1085)
* `nowallbug` worldspawn key to prevent acceleration while stuck in a wall [#1155](https://github.com/etjump/etjump/pull/1155)
* `target_portal_relay` entity - activate targets based on players shot portal count [#1130](https://github.com/etjump/etjump/pull/1130)
* `etj_fixedCushionSteps` to play proper fall step sounds on `surfaceparm nodamage` [#1136](https://github.com/etjump/etjump/pull/1136)
* `ob_list` to display saved OB watcher coordinates [#1091](https://github.com/etjump/etjump/pull/1091)
* `noise` & `volume` key support for `target_remove_portals` [#1129](https://github.com/etjump/etjump/pull/1129) [#1130](https://github.com/etjump/etjump/pull/1130)
* `spawnflag 2` for `target_remove_portals` to fire targets when portals are removed [#1129](https://github.com/etjump/etjump/pull/1129) [#1130](https://github.com/etjump/etjump/pull/1130)
* `customvotes` alias for `listinfo` [#1152](https://github.com/etjump/etjump/pull/1152)
* `readCustomvotes` command to read customvote file (useful after manual editing) [#1152](https://github.com/etjump/etjump/pull/1152)

## Changed
* `etj_drawCGaz` is now a bitflag cvar and allows drawing both CGaz HUDs simultaneously [#1137](https://github.com/etjump/etjump/pull/1137)
* timerun start now only clears saves made during a timerun [#1149](https://github.com/etjump/etjump/pull/1149)
  * any save slots made outside of a timerun will remain valid, loading will interrupt the timerun
  * backups are cleared of any backups made from timerun saves, and valid slots are pushed to the front of the queue
* `players` command output lists the game engine used by each client [#1089](https://github.com/etjump/etjump/pull/1089)
* timeruns cannot be started if `pmove_fixed 0` was set after spawning [#1156](https://github.com/etjump/etjump/pull/1156)
  * does not apply to timeruns which do not force `pmove_fixed 1`
* `map` parameter in `records` is no longer case sensitive, and supports partial matching [#1119](https://github.com/etjump/etjump/pull/1119) [#1148](https://github.com/etjump/etjump/pull/1148)
* inactive specators draw semi transparent in spectator list [#1116](https://github.com/etjump/etjump/pull/1116)
* portalgun portals of spectated player are now draw with normal portal colors instead of "others" colors [#1132](https://github.com/etjump/etjump/pull/1132)
* `snaps` value in `players` output is colored red if it's less than `sv_fps` [#1089](https://github.com/etjump/etjump/pull/1089)
* improved `!seasons` command output [#1118](https://github.com/etjump/etjump/pull/1118)
  * season list is grouped to active, upcoming and past seasons
  * `Default` season is no longer displayed
  * start and end date format changed for better legibility
* votes are canceled if the caller disconnects [#1134](https://github.com/etjump/etjump/pull/1134)
* most OB watcher prints are now popups instead of console prints [#1091](https://github.com/etjump/etjump/pull/1091)
* `target_remove_portals` no longer displays a print if no portals were removed [#1129](https://github.com/etjump/etjump/pull/1129)
* replay menu strips color codes from demo filenames [#1115](https://github.com/etjump/etjump/pull/1115)

## Removed
* `target_printname` entity, all functionality is included in `target_print` [#1080](https://github.com/etjump/etjump/pull/1080)
* `trigger_multiple spawnflags 512/2048` [#1144](https://github.com/etjump/etjump/pull/1144)
  * `spawnflag 512` never provided anything unique, it was just a shorthand for setting `wait` to one server frame
  * `spawnflag 2048` is now default behavior for all triggers
* cleaned up several unused server cvars [#1111](https://github.com/etjump/etjump/pull/1111)

## Fixed
* `cg_uinfo` was not set correctly on init on 64-bit ETe clients on Windows [#1086](https://github.com/etjump/etjump/pull/1086)
* entering wounded state was not possible during timeruns (broke the timerun in `2weeks` for example) [#1126](https://github.com/etjump/etjump/pull/1126)
* team/fireteam chat closing parenthesis/bracket was affected by player name colors [#1084](https://github.com/etjump/etjump/pull/1084)
* `records <map> <run>` failed to get results if `map` was partially matching another map [#1148](https://github.com/etjump/etjump/pull/1148)
* jump speeds were truncated instead of rounded, now matches other speed displays [#1150](https://github.com/etjump/etjump/pull/1150)
* portal shader size did not match the actual portal size properly [#1133](https://github.com/etjump/etjump/pull/1133)
* crash with `shooter_grenade` entity [#1093](https://github.com/etjump/etjump/pull/1093)
* `backup` queue broke on initial save to a specific save slot (`save 0/1/2`) [#1145](https://github.com/etjump/etjump/pull/1145)
* casting multiple votes was possible by reconnecting [#1134](https://github.com/etjump/etjump/pull/1134)
* client vote info (e.g. re-vote counts) were not reset when a vote failed/was canceled [#1142](https://github.com/etjump/etjump/pull/1142)
* `ob_save [name]` and `ob_load [name]` were not working [#1091](https://github.com/etjump/etjump/pull/1091)
* OB watcher was ignoring surfaces that don't allow overbounces [#1091](https://github.com/etjump/etjump/pull/1091)
* OB watcher coordinate prints used player origin instead of player feet as Z coordinate [#1091](https://github.com/etjump/etjump/pull/1091)
* performance optimization for some HUD elements in spec/demo playback, up to ~30% faster in some scenarios [#1109](https://github.com/etjump/etjump/pull/1109)
* crash with `await` command when `frames` argument was out of range [#1120](https://github.com/etjump/etjump/pull/1120)
* crash with `target_activate` when activated without activator [#1093](https://github.com/etjump/etjump/pull/1093)
* crash with `setplayerspawn`, `setplayerautospawn`, `killplayer` and `damageplayer` script actions when activated without activator [#1093](https://github.com/etjump/etjump/pull/1093)
* portalgun bypassed checks for `g_weapons` and `g_knifeOnly` [#1121](https://github.com/etjump/etjump/pull/1121)
* friction and look sensitivity were incorrect in freecam [#1088](https://github.com/etjump/etjump/pull/1088)

# ETJump 3.0.1

## Added
* `etj_CGaz2NoVelocityDir` cvar to hide velocity direction line on CGaz 2 [#1074](https://github.com/etjump/etjump/pull/1074)
* support value **-1** for `rank` in `loadcheckpoints` to "unload" checkpoints [#1077](https://github.com/etjump/etjump/pull/1077)
* `rankings <season>` command shorthand [#1073](https://github.com/etjump/etjump/pull/1073)
* server logs all maps to `maps.json` on game init [#1070](https://github.com/etjump/etjump/pull/1070)

## Changed
* runtimer and checkpoint timer accuracy improved while playing, spec/demo playback are still interpolated [#1069](https://github.com/etjump/etjump/pull/1069)
* `loadcheckpoints` supports partial name matching [#1077](https://github.com/etjump/etjump/pull/1077)
* `!delete-season` command is no longer case sensitive [#1058](https://github.com/etjump/etjump/pull/1058)
* `rankings` excludes players with no score [#1078](https://github.com/etjump/etjump/pull/1078)
* runtimer color transition now starts at the last 10% of the run (upper bound to 10s) [#1071](https://github.com/etjump/etjump/pull/1071)
* `listinfo` no longer prints "missing maps" section if no maps are missing [#1062](https://github.com/etjump/etjump/pull/1062)

## Fixed
* relative checkpoint timer (`etj_checkpointsStyle 0`) had 1ms error when player had no record on a run [#1066](https://github.com/etjump/etjump/pull/1066)
* `etj_ad_savePBOnly 1` did not save the demo if a new PB was made, but it wasn't a new overall record [#1075](https://github.com/etjump/etjump/pull/1075)
* checkpoint timer broke when player hit the max checkpoint limit [#1071](https://github.com/etjump/etjump/pull/1071)
* record banner was not shown if player beat the seasonal or overall record, and had no prior record on a run [#1075](https://github.com/etjump/etjump/pull/1075)
* record banner prints were using wrong time for diff calculation [#1075](https://github.com/etjump/etjump/pull/1075)
* `records` without arguments was printing records from maps which partially matched the current map name [#1063](https://github.com/etjump/etjump/pull/1063)
* seasonal record banner prints were missing parenthesis around the diff [#1057](https://github.com/etjump/etjump/pull/1057)
* checkpoint timer was incorrectly positioned on non-default size [#1071](https://github.com/etjump/etjump/pull/1071)
* checkpoint timer was not fading out with runtimer [#1071](https://github.com/etjump/etjump/pull/1071)
* `--page-size` argument on `records` was not working in some scenarios [#1059](https://github.com/etjump/etjump/pull/1059)
* new PB times for timeruns were not printing to all players [#1061](https://github.com/etjump/etjump/pull/1061)
* `!edit-season` could not edit start and end dates with a single command [#1079](https://github.com/etjump/etjump/pull/1079)

# ETJump 3.0.0

## Breaking
* reworked `g_blockCheatCvars` to be a bitflag cvar [#1016](https://github.com/etjump/etjump/pull/1016)

## Added
* new timerun system with checkpoint support, seasons and score rankings [#995](https://github.com/etjump/etjump/pull/995) [#1014](https://github.com/etjump/etjump/pull/1014) [#1017](https://github.com/etjump/etjump/pull/1017) [#1020](https://github.com/etjump/etjump/pull/1020) [#1021](https://github.com/etjump/etjump/pull/1021) [#1043](https://github.com/etjump/etjump/pull/1043) [#1048](https://github.com/etjump/etjump/pull/1048) [#1050](https://github.com/etjump/etjump/pull/1050)
  * `etj_drawCheckpoints` & related cvars to control checkpoint drawing
  * `target_checkpoint` and `trigger_checkpoint` entities
  * `trigger_starttimer` & `trigger_stoptimer` entities
  * `_ext` versions of all timerun trigger entities for mapscripting
  * improved records printing (`records/ranks/times/top`)
    * ability to query records from any map on the server
    * `records` command can be used as admin command `!records` (flag `a`)
    * all parameters support partial matching
  * `g_timeruns2Database` cvar to set new timerun database name on server (default `timeruns.v2.db`)
    * old timerun database will automatically be migrated to the new database format
* edge-only snaphud (`etj_drawSnapHUD 2`) - draw only the end points of snap zones [#1037](https://github.com/etjump/etjump/pull/1037)
  * `etj_snaoHUDEdgeThickness` controls the drawing thickness (**1 - 128**)
* support `autoexec_default` on client - executed if `autoexec_mapname` isn't present [#1023](https://github.com/etjump/etjump/pull/1023)
* `etj_noPanzerAutoswitch` cvar to disable autoswitch after firing a panzerfaust [#1029](https://github.com/etjump/etjump/pull/1029)
  * switch still occurs if completely out of ammo
* fireteam savelimit controls in fireteam admin menu via new `Rules` page [#1026](https://github.com/etjump/etjump/pull/1026)
* fireteam savelimit indicator in fireteam UI [#1026](https://github.com/etjump/etjump/pull/1026)
* bbox drawing for following entities when `g_scriptDebug 1` is enabled [#1042](https://github.com/etjump/etjump/pull/1042) [#1048](https://github.com/etjump/etjump/pull/1048)
  * `trigger_multiple_ext` (green)
  * `trigger_starttimer_ext` (blue)
  * `trigger_stoptimer_ext` (blue)
  * `trigger_checkpoint_ext` (magenta)
  * `func_fakebrush` (red)
* bundled fixed mapscripts for `branx`, `branx2` and `freestyle` with the mod so they work out of box without a custom script [#1039](https://github.com/etjump/etjump/pull/1039)

## Changed
* `records <run>` now shows top 20 times by default, instead of top 50 [#995](https://github.com/etjump/etjump/pull/995)
* fireteam savelimit default changed to **-1** (unlimited), **0** disables saving [#1026](https://github.com/etjump/etjump/pull/1026)
* fireteam UI no longer draws ranks and locations [#1026](https://github.com/etjump/etjump/pull/1026)
* fireteam UI no longer draws class string for spectators [#1026](https://github.com/etjump/etjump/pull/1026)
* limbo menu camera now tries to display view from spectator spawn point if no limbo cameras are present in the map [#1040](https://github.com/etjump/etjump/pull/1040)
  * only works if the spectator spawn point is in current PVS

## Fixed
* actions executed on `load` (e.g. autodemo recording) were executing on wrong clients [#1041](https://github.com/etjump/etjump/pull/1041)
* `g_blockedMaps` was blocking maps by partial name match [#1035](https://github.com/etjump/etjump/pull/1035)
* spectator list was not hidden when scoreboard was visible [#1024](https://github.com/etjump/etjump/pull/1024)
* picking up weapons was impossible after they were removed by `target_init` [#1028](https://github.com/etjump/etjump/pull/1028)
* cvar unlockers were de-synced after map restart [#1032](https://github.com/etjump/etjump/pull/1032)
* fireteam savelimit was not correctly communicated to all fireteam members [#1026](https://github.com/etjump/etjump/pull/1026)
* `etj_fireteamAlpha` was partially ignored for fireteam UI health display [#1026](https://github.com/etjump/etjump/pull/1026)

# ETJump 2.6.0

## Added
* 7 new crosshairs drawn as vector graphics [#993](https://github.com/etjump/etjump/pull/993)
  * `etj_crosshairScaleX/Y` to scale on x/y (also works for old crosshairs, range **-5 - 5**)
  * `etj_crosshairThickness` to set line thickness (**0 - 5**)
  * `etj_crosshairOutline` toggles drawing outline with the new crosshairs
* `etj_CGaz2FixedSpeed` cvar to allow specifying fixed speed for drawing CGaz 2 [#855](https://github.com/etjump/etjump/pull/855)
* cheat status indicator on scoreboard(s) [#863](https://github.com/etjump/etjump/pull/863)
* `etj_jumpSpeedsMinSpeed` for setting a minimum threshold for jump speeds list [#866](https://github.com/etjump/etjump/pull/866)
  * jump speeds under this value are colored with `etj_jumpSpeedsSlowerColor`
  * works independently of `etj_jumpSpeedsShowDiff`
* `etj_menuSensitivity` for controlling mouse sensitivity in UI/menus [#931](https://github.com/etjump/etjump/pull/931)
* warning print when starting a timerun if cheats are enabled (displays once per map) [#924](https://github.com/etjump/etjump/pull/924)
* `etj_drawSpectatorInfo` can be drawn center or right aligned with values **2** and **3**, respectively [#964](https://github.com/etjump/etjump/pull/964)
* `etj_spectatorInfoSize` cvar to change spectator info text size [#964](https://github.com/etjump/etjump/pull/964)
* `etj_spectatorInfoShadow` cvar to enable shadowed text on spectator info list [#964](https://github.com/etjump/etjump/pull/964)
* `etj_footstepVolume` to control volume of movement-generated sounds (footsteps & landings) [#938](https://github.com/etjump/etjump/pull/938)
* `nodrop` worldspawn key to globally enable nodrop in the map (reverses `surfaceparm nodrop` behavior) [#986](https://github.com/etjump/etjump/pull/986)
* banner print when a new #1 record is made on a timerun [#935](https://github.com/etjump/etjump/pull/935)
* warning print when server is started on ET: Legacy or ETe with patch collision fix [#967](https://github.com/etjump/etjump/pull/967)
* `autoexec_axis/allies/spectator` support [#867](https://github.com/etjump/etjump/pull/867)
* `etj_flareSize` cvar unlocker for `r_flareSize` [#925](https://github.com/etjump/etjump/pull/925)
* `etj_clear` cvar unlocker for `r_clear` [#856](https://github.com/etjump/etjump/pull/856)
* `listspawnpt` command to list valid spawnpoints in a map [#933](https://github.com/etjump/etjump/pull/933)
* `top` alias for viewing timerun records [#884](https://github.com/etjump/etjump/pull/884)
* `cv` alias for `callvote` [#902](https://github.com/etjump/etjump/pull/902)
* automatic versioning for credits menu in non-release builds [#910](https://github.com/etjump/etjump/pull/910)

## Changed
* `load` skips death cam (small period after getting gibbed) and respawns you immediately [#962](https://github.com/etjump/etjump/pull/962)
* timeruns no longer start if a client has used `noclip` after respawning [#998](https://github.com/etjump/etjump/pull/998)
* reversed diff column on `records` output - faster records are now green with `-`, slower red with `+` (relative to your time) [#935](https://github.com/etjump/etjump/pull/935)
* `etj_optimizePrediction` cvar - improves performance of playerstate prediction [#916](https://github.com/etjump/etjump/pull/916)
* listbox UI menus (demo, mods, mapvote etc.) now have shadowed text [#857](https://github.com/etjump/etjump/pull/857)
* `cg_lagometer 1` now draws lagometer only on online servers again, `cg_lagometer 2` enables drawing on localhost [#912](https://github.com/etjump/etjump/pull/912)
* `etj_spectatorInfoY` default changed from **40** to **30** [#964](https://github.com/etjump/etjump/pull/964)
  * value now indicates the top left of the drawing instead of bottom left of the first name
* quadrupled the number of supported maps by the mod, should be roughly 2000 maps now [#990](https://github.com/etjump/etjump/pull/990)
* increased `!newmaps` max output to 50 maps [b38223a](https://github.com/etjump/etjump/commit/b38223a0bd7f12eaf45103b159ad4ef534774b25)
* limited `!most/leastplayed` output to 100 maps [#865](https://github.com/etjump/etjump/pull/865)
* invalid arguments for `vote` now display help text instead of defaulting to `vote no` [#906](https://github.com/etjump/etjump/pull/906)
* `vote` arguments are now case insensitive and ignore color codes [#906](https://github.com/etjump/etjump/pull/906)
* increased UI memory pool size for 64-bit clients [#875](https://github.com/etjump/etjump/pull/875)
* updated viewlog menu tooltip to inform viewlog usage on ET: Legacy clients [1bd5f8b](https://github.com/etjump/etjump/commit/1bd5f8b6c363ddf17df366462afe9462e07831d7)
* `etj_nofatigue` no longer changes stamina bar visuals, color pulsing only happens when using adrenaline [#922](https://github.com/etjump/etjump/pull/922)
* `cg_runroll` and `cg_runpitch` default values changed to **0** [#987](https://github.com/etjump/etjump/pull/987)
* better formatting for `listinfo` output [#996](https://github.com/etjump/etjump/pull/996)
* mission timer and spawntime no longer display in limbo menu [#892](https://github.com/etjump/etjump/pull/892)
* rephrased some vote related messages and moved some from console prints to popups [#876](https://github.com/etjump/etjump/pull/876)
* `entitylist` displays used/total and inactive/used entities [#954](https://github.com/etjump/etjump/pull/954)
* vsay prints in console are no longer prefixed with a colon [9aa7acb](https://github.com/etjump/etjump/commit/9aa7acbd99daf5ba04f299d92a3073bf97490923)
* some unnecessary cvars are no longer part of serverinfo [#885](https://github.com/etjump/etjump/pull/885) [#889](https://github.com/etjump/etjump/pull/889)

## Removed
* `!nogoto`, `!nosave`, `!putteam` and `!rmsaves` admin commands [#869](https://github.com/etjump/etjump/pull/869)
* mapscript actions `cvar set/inc/random` [#934](https://github.com/etjump/etjump/pull/934)
* various console commands from tab completion that did nothing [#994](https://github.com/etjump/etjump/pull/994)
* `cg_predictItems` cvar (unused) [#900](https://github.com/etjump/etjump/pull/900)

## Fixed
* collision with players and other entities was broken with `pmove_fixed 1` when playing at > 125FPS [#880](https://github.com/etjump/etjump/pull/880)
* strafe quality meter was not correctly detecting when the player was on ground, dead, noclipping, or mounted on MG/mortar [#833](https://github.com/etjump/etjump/pull/883)
* upmove meter was not correctly detecting when the player was dead, noclipping or mounted on MG/mortar [#833](https://github.com/etjump/etjump/pull/883)
* portals could be shot on players' heads [#957](https://github.com/etjump/etjump/pull/957)
* timerun timer disappeared on demo playback when timerun ended [#835](https://github.com/etjump/etjump/pull/853)
* `records <run>` was not working if the run name has color codes [#904](https://github.com/etjump/etjump/pull/904)
* drowning was inconsistent `pmove_fixed 1` when playing at > 125FPS [#984](https://github.com/etjump/etjump/pull/984)
* `tracker_not_eq` key would fire targets if a single value was not matching, instead of requiring all values to not match [#997](https://github.com/etjump/etjump/pull/997)
* multiple votes could be cast by switching teams [#864](https://github.com/etjump/etjump/pull/864)
* upmove meter graph was not widescreen corrected [d04378c](https://github.com/etjump/etjump/commit/d04378c93775193c7c52d3c73ee4e816b3077d2b)
* chatbox ("send message" window) lines would sometimes overlap [#899](https://github.com/etjump/etjump/pull/899)
* cvar unlockers got de-synced on map changes with ET: Legacy and ETe clients [#870](https://github.com/etjump/etjump/pull/870)
* scoreboard inactivity icon was not persisting across map changes [#906](https://github.com/etjump/etjump/pull/907) [#913](https://github.com/etjump/etjump/pull/913)
* green/red colors in runtimer at timerun end were not working if the run name contained whitespace [#941](https://github.com/etjump/etjump/pull/941)
* portalgun portals were always rendered and were sometimes visible unexpectedly due to shader sorting order [#949](https://github.com/etjump/etjump/pull/949)
* vote UI showed selection around "no" when the player voted "yes" as a spectator and was following someone [#882](https://github.com/etjump/etjump/pull/882)
* long names could cause an overlap in scoreboard 3 when following others [#862](https://github.com/etjump/etjump/pull/862)
* scoreboard icons were not correctly fading out with the rest of the scoreboard [#908](https://github.com/etjump/etjump/pull/908)
* potential crash when activating `trigger_multiple` with a non-player entity [#920](https://github.com/etjump/etjump/pull/920)
* portalgun trails were not respecting `etj_viewPlayerPortals` [#959](https://github.com/etjump/etjump/pull/959)
* CGaz 2 was not correctly centered on crosshair [#966](https://github.com/etjump/etjump/pull/966) [#999](https://github.com/etjump/etjump/pull/999)
* certain indicators were inaccurate on demo playback [#992](https://github.com/etjump/etjump/pull/992)
* `fireteam invite/warn/kick/propose` command handling issues [#928](https://github.com/etjump/etjump/pull/928)
  * action by clientnum was sending command to previous client number
  * action by clientname now returns invalid clientnum and errors if matching multiple clients
* flamethrower spawned too many flames with `cg_drawGun 0` when repeatedly tapping `+attack` [#919](https://github.com/etjump/etjump/pull/919)
* fireteam admin menu was drawing behind some HUD elements [#936](https://github.com/etjump/etjump/pull/936)
* jittery view when riding on movers with > 125FPS [#916](https://github.com/etjump/etjump/pull/916)
* landing view animation did not reset when switching teams in the middle of the animation [#916](https://github.com/etjump/etjump/pull/916)
* joining spectators while proning would play a view transition, as if the player was standing up [#916](https://github.com/etjump/etjump/pull/916)
* prediction errors were not cleared on respawn [#916](https://github.com/etjump/etjump/pull/916)
* `say_buddy` and `enc_say*` commands were missing from tab completion [#994](https://github.com/etjump/etjump/pull/994)
* `entitylist` was missing from tab completion [#954](https://github.com/etjump/etjump/pull/954)
* `g_ghostplayers 0` caused players to get stuck on each other when spawning on same spawnpoint [#879](https://github.com/etjump/etjump/pull/879)
* some weapon icons were affected by `r_picmip` [cc82a07](https://github.com/etjump/etjump/commit/cc82a073bcb12482229d0675c1a62dde795d0778)
* player icon on map was warping when trying to `noclip` while mounted in a tank [#903](https://github.com/etjump/etjump/pull/903)
* incorrect error message when `misc_gamemodel` was used with `spawnflag 2` and `frames` were set to 0 [89d7f32](https://github.com/etjump/etjump/commit/89d7f320d0b9c925be51f55379f29da72d3b2483)

# ETJump 2.5.1

## Added
* `etj_CGazOnTop` cvar to draw CGaz on top of snaphud [#848](https://github.com/etjump/etjump/pull/848)

## Changed
* `g_oss` default changed to reflect Win x64/M1 Mac support [e5f6a79](https://github.com/etjump/etjump/commit/e5f6a79402c56bbe655eac6cc4ef27b533b08954)
* `g_oss` is now read only (not meant to be set by server admins) [#850](https://github.com/etjump/etjump/pull/850)
* renamed `savelimit` worldspawn key to `limitedsaves` to avoid issues with old maps which might have set the key prior to it working in 2.5.0 [#841](https://github.com/etjump/etjump/pull/841)

## Fixed
* occasional crash when switching teams or loading a new map with when some HUD elements enabled [#835](https://github.com/etjump/etjump/pull/835) [#844](https://github.com/etjump/etjump/pull/844)
* crash with ETe in localhost when changing a map while in a team [#835](https://github.com/etjump/etjump/pull/835)
* player weapon was swapped briefly to primary weapon when starting a timerun and not holding the primary weapon [#832](https://github.com/etjump/etjump/pull/832)
* jump speeds, strafe quality and upmove meters would not sometimes reset on death [#834](https://github.com/etjump/etjump/pull/834)
* view transitions played when `load` forced a different stance [#837](https://github.com/etjump/etjump/pull/837)
* `etj_speedColorUsesAccel` did not respect `etj_speedAlpha` [#840](https://github.com/etjump/etjump/pull/840)
* scoreboard was displaying FPS as **0** when client had `com_maxFPS` set to > 998 [#849](https://github.com/etjump/etjump/pull/849)

# ETJump 2.5.0

## Breaking
* removed CGaz HUDs **1**, **3** and **4** - old CGaz **5** is now CGaz **1** [#666](https://github.com/etjump/etjump/pull/666)
* renamed several CGaz-related cvars [#666](https://github.com/etjump/etjump/pull/666)
  * `etj_CGazColor1` -> `etj_CGaz2Color1`
  * `etj_CGazColor2` -> `etj_CGaz2Color2`
  * `etj_CGaz5Color1` -> `etj_CGaz1Color1`
  * `etj_CGaz5Color2` -> `etj_CGaz1Color2`
  * `etj_CGaz5Color3` -> `etj_CGaz1Color3`
  * `etj_CGaz5Color4` -> `etj_CGaz1Color4`
  * `etj_CGaz5Fov` -> `etj_CGazFov`
* replaced `etj_weaponSound` with `etj_weaponVolume` to allow adjusting volume of weapon sounds [#570](https://github.com/etjump/etjump/pull/570)

## Added
* `etj_drawJumpSpeeds` cvar to display last 10 jump speeds [#608](https://github.com/etjump/etjump/pull/608) [#803](https://github.com/etjump/etjump/pull/803)
* `etj_drawStrafeQuality` cvar to display percentage of physics frames spent on optimal accel zone [#641](https://github.com/etjump/etjump/pull/641)
* `etj_drawUpmoveMeter` cvar to display how long `+moveup` is being held for while jumping [#802](https://github.com/etjump/etjump/pull/802)
* new portalgun model and portal shaders [#787](https://github.com/etjump/etjump/pull/787) [#822](https://github.com/etjump/etjump/pull/822)
* macOS support (10.10 and up, only x86_64) [#590](https://github.com/etjump/etjump/pull/590), [#595](https://github.com/etjump/etjump/pull/595)
* `etj_snapHUDHLActive` cvar - highlight active snapzone [#666](https://github.com/etjump/etjump/pull/666)
  * `etj_snapHUDHLColor1/2` to set colors
* `etj_CGazTrueness` cvar to set whether certain physics aspects are reflected in CGaz drawing [#666](https://github.com/etjump/etjump/pull/666)
* `etj_snapHUDTrueness` cvar to set whether certain physics aspects are reflected in snaphud drawing [#666](https://github.com/etjump/etjump/pull/666)
* `etj_projection` cvar to set projection type for CGaz 1 and snaphud (rectilinear, cylindrical or panini) [#666](https://github.com/etjump/etjump/pull/666)
* `target_init` entity - initializes player to a freshly spawned state [#596](https://github.com/etjump/etjump/pull/596)
* CHS info **55** to display last jump speed [#603](https://github.com/etjump/etjump/pull/603)
* cvar unlockers for ETe clip/trigger/slick drawing cvars [#813](https://github.com/etjump/etjump/pull/813)
  * requires ETe build from Sep 10th 2022 or newer
  * alternative shaders included in the mod to replace built-in ones from engine
* support for `callvote devmap` to vote a map with cheats [#814](https://github.com/etjump/etjump/pull/814)
* `!newmaps [N]` admin command to display `N` latest maps added to server (admin flag `a`) [#825](https://github.com/etjump/etjump/pull/825)
* `etj_saveMsg` cvar to set save message on client side [#729](https://github.com/etjump/etjump/pull/729)
* `etj_speedAlign` to control speed meter alignment [#561](https://github.com/etjump/etjump/pull/561)
* `spawnflag 8` for `func_static` to allow disabling random 500-1000ms delay added between activations [#582](https://github.com/etjump/etjump/pull/582)
* timerun status indicator in scoreboard [#566](https://github.com/etjump/etjump/pull/566), [#568](https://github.com/etjump/etjump/pull/568)
* idle indicator in scoreboard [#568](https://github.com/etjump/etjump/pull/568)
* `unload` command to revert last `load` command [#569](https://github.com/etjump/etjump/pull/569)
  * cannot be used during timeruns or to teleport to areas where you cannot use `save`
  * only **1** unload slot, overwritten on every successful `load` command
* `etj_drawBanners` to allow hiding banners [#587](https://github.com/etjump/etjump/pull/587) [#805](https://github.com/etjump/etjump/pull/805)
* binocular-like zoom for spectators using `+zoom` [#601](https://github.com/etjump/etjump/pull/601)
* built-in default command map shader, displayed when map does not include a command map [#800](https://github.com/etjump/etjump/pull/800)
* `etj_fixedCompassShader` cvar - modifies command map shader to ensure proper masking with compass [#800](https://github.com/etjump/etjump/pull/800)
  * can break some complex command map shaders
* `g_oss` server cvar to improve integration with ET: Legacy server browser [#665](https://github.com/etjump/etjump/pull/665)
* `etj_expandedMapAlpha` to control transparency of expanded map [#771](https://github.com/etjump/etjump/pull/771)
* silent command flag `/` to `!listflags` output [#557](https://github.com/etjump/etjump/pull/557)
* `etj_FPSMeterUpdateInterval` to control the update rate of FPS meter [#793](https://github.com/etjump/etjump/pull/793)
* `noise` key support for `func_button` [#580](https://github.com/etjump/etjump/pull/580)
* several new binding options to ETJump binds menu [#774](https://github.com/etjump/etjump/pull/774)
* reintroduced Quake 3 `constantLight` support [#579](https://github.com/etjump/etjump/pull/579)
  * `light <value>` and `color <R G B>` to make brush entities emit light

## Changed
* several map restrictions no longer apply when cheats are enabled [#814](https://github.com/etjump/etjump/pull/814)
  * save, noclip, god and prone are not restricted
  * cheats are enabled during timeruns (no records are saved)
  * save is available during nosave timeruns, load doesn't reset the run
* portalgun range increased from **5000** units to max map size [#824](https://github.com/etjump/etjump/pull/824)
* removed sway from binoculars [#601](https://github.com/etjump/etjump/pull/601)
* CGaz and snaphud no longer draw while noclipping, dead, on ladders, underwater or using mounted/set weapons [#666](https://github.com/etjump/etjump/pull/666) [#686](https://github.com/etjump/etjump/pull/686)
* CGaz and snaphud no longer draw when zooming in with binoculars or scoped weapons [#601](https://github.com/etjump/etjump/pull/601)
* crosshair is no longer hidden while moving in prone [#744](https://github.com/etjump/etjump/pull/744)
* setting `com_maxFPS` < 25 interrupts an ongoing timerun [#602](https://github.com/etjump/etjump/pull/602)
* `cg_drawRoundTimer` displays the elapsed time of current map [#791](https://github.com/etjump/etjump/pull/791)
* dead players cannot use `noclip` [#714](https://github.com/etjump/etjump/pull/714)
* `speclock` and spec invites persist across map changes [#769](https://github.com/etjump/etjump/pull/796)
* field ops now spawn with binoculars [#593](https://github.com/etjump/etjump/pull/593)
  * calling artillery is only possible when cheats are enabled
* `etj_speedSize` now accepts decimal values [#617](https://github.com/etjump/etjump/pull/617)
* command map no longer draws a grid [#726](https://github.com/etjump/etjump/pull/726)
* tied timeruns completion messages draw in white instead of red [#658](https://github.com/etjump/etjump/pull/658)
* `!listmaps` displays total number of maps on server [#588](https://github.com/etjump/etjump/pull/588)
* `etj_CGaz1Color1-4` (old `etj_CGaz5Color1-4`) default alpha values changed from **1.0** to **0.75**
* `etj_CGazY` default value changed from **260** to **240**
* slightly increased `etj_altScoreboard 3` size [#673](https://github.com/etjump/etjump/pull/673)
* `!listmaps` now defaults to 5 columns instead of 3 [#825](https://github.com/etjump/etjump/pull/825)
  * max columns capped to 5 to preserve console output formatting
* `!listmaps` and `!findmap` outputs are sorted alphabetically, and every other column colored grey to improve readability [#825](https://github.com/etjump/etjump/pull/825)
* `!findmap` displays the number of matches [#825](https://github.com/etjump/etjump/pull/825)
* silent command execution permission is now visible in `!help` output [#557](https://github.com/etjump/etjump/pull/557)
* clamped `scale` value with `target_scale_velocity` and `spawnflag 1` to **0.25 - 3.0** range [#687](https://github.com/etjump/etjump/pull/687)
* spectators can no longer use `spec(un)lock` [#619](https://github.com/etjump/etjump/pull/619)
* `etj_drawOB 2` skips sticky OB drawing on same height as the player is at [#600](https://github.com/etjump/etjump/pull/600)
* `g_mapScriptDir` default changed from `scripts` to `mapscripts` [#804](https://github.com/etjump/etjump/pull/804)
* removed cheat protection from `cg_skybox` [#776](https://github.com/etjump/etjump/pull/776)
* removed `Objective:` text from loading screen and made mapname font size to scale automatically to the boundaries of the box [#770](https://github.com/etjump/etjump/pull/770)
* expanded map always draws on top of HUD [#630](https://github.com/etjump/etjump/pull/630)
* improved command map accuracy [#734](https://github.com/etjump/etjump/pull/734)
* binocular/scope/mortar reticles are hidden while scoreboard is up [#794](https://github.com/etjump/etjump/pull/794)
* compass is hidden while scoreboard is up [#794](https://github.com/etjump/etjump/pull/794)
* text shadowing is more consistent across various text sizes [#816](https://github.com/etjump/etjump/pull/816)
* revive icon alpha value is no longer tied to spawntimes [#718](https://github.com/etjump/etjump/pull/718)
* `cg_descriptiveText 0` now hides reinforcement timer [#612](https://github.com/etjump/etjump/pull/612)
* `g_password` is no longer stored userinfo string [#682](https://github.com/etjump/etjump/pull/682)
* `g_maxConnsPerIP` ignores negative values [#807](https://github.com/etjump/etjump/pull/807)
* renamed `sv_mapname` to `mapname` (server cvar indicating current map on the server) [#764](https://github.com/etjump/etjump/pull/764)
* timescale no longer resets after demo playback or `vid_restart` [#599](https://github.com/etjump/etjump/pull/599)

## Removed
* `g_saveMsg` server cvar [#729](https://github.com/etjump/etjump/pull/729)
* `shared`, `cg_synchronousClients` and `pmove_msec` cvars from cgame cvars, handled directly via systeminfo configstrings [#762](https://github.com/etjump/etjump/pull/762)
* `pmove_fixed` cvar from server [#764](https://github.com/etjump/etjump/pull/764)
* `etj_CGazWidth` and `etj_CGazAlpha` (no longer used) [#666](https://github.com/etjump/etjump/pull/666)

## Fixed
* jumping twice on NJD surface bypassed jump delay on initial landing on a regular surface [#622](https://github.com/etjump/etjump/pull/622)
* CGaz calculated incorrect angles while ground strafing [#635](https://github.com/etjump/etjump/pull/635)
* tied votes passed instead of failed with `vote_percent 50` [#560](https://github.com/etjump/etjump/pull/560)
* `speclock` could be unintentionally bypassed if a spec invited client disconnected, and another client connected using the same slot [#789](https://github.com/etjump/etjump/pull/789)
* 64-bit servers crashed on some maps due to memory pool being too small [#654](https://github.com/etjump/etjump/pull/654)
* `shared` cvar was setup incorrectly, causing issues with ET: Legacy and ETe clients [#701](https://github.com/etjump/etjump/pull/701)
* map restarts played a brief warmup before fully restarting [#815](https://github.com/etjump/etjump/pull/815)
* color cvars displayed incorrectly in menus on 64-bit clients [#626](https://github.com/etjump/etjump/pull/626) [#629](https://github.com/etjump/etjump/pull/629)
* covert ops weapons were not correctly handled on timerun start [#598](https://github.com/etjump/etjump/pull/598)
* weapon switch animation played when picking up a portalgun [#577](https://github.com/etjump/etjump/pull/577)
* portalgun pickup was missing a print and sound [#577](https://github.com/etjump/etjump/pull/577)
* `!listmaps` and `!map` ignored `g_blockedMaps` [#669](https://github.com/etjump/etjump/pull/669)
* runtimer looped back to **00:** if a run took more than 1 hour [#567](https://github.com/etjump/etjump/pull/576)
* `savelimit` worldspawn key was not working [#827](https://github.com/etjump/etjump/pull/827)
* several weapon related sounds were not affected by `etj_weaponVolume` [#694](https://github.com/etjump/etjump/pull/694)
  * reload & switch sounds
  * satchel detonator idle sound
  * panzer/satchel detonator spin-up
  * mounted MG42s and tanks
  * overheating sounds
  * dynamite spin-up
* item pickup sounds were not playing with `etj_itemPickupText 0` [#692](https://github.com/etjump/etjump/pull/692)
* knockback and view duck were not correctly predicted when taking fall damage [#812](https://github.com/etjump/etjump/pull/812)
* `speclist` was missing from tab completion [#619](https://github.com/etjump/etjump/pull/619)
* view transitioned from crouch to prone played when following prone players [#715](https://github.com/etjump/etjump/pull/715)
* player view persistently shifted slightly after step-ups and hard landings upon following [#717](https://github.com/etjump/etjump/pull/717)
* potential crash in map grid code with invalid map coordinates [#716](https://github.com/etjump/etjump/pull/716)
* sounds for client 0 played too far away on some maps with OpenAL sound backend [#644](https://github.com/etjump/etjump/pull/644)
* server and map name could overlap scoreboard 3 [#673](https://github.com/etjump/etjump/pull/673)
* default scoreboard header spelled playercount wrong when a team had one player [#565](https://github.com/etjump/etjump/pull/565)
* configstrings limit ignored fireteam configstrings [#725](https://github.com/etjump/etjump/pull/725)
* multi-line tooltips in menus had inconsistent line heights [#788](https://github.com/etjump/etjump/pull/788)
* `team` command printed duplicate spam protection prints [#591](https://github.com/etjump/etjump/pull/591)
* demo playback allowed several commands that caused unwanted behavior if executed [#757](https://github.com/etjump/etjump/pull/757)
* binocular/scope overlay disappeared while scoreboard was up [#794](https://github.com/etjump/etjump/pull/794)
* speedmeter could show 1ups speed while wounded on a non-flat surface [#731](https://github.com/etjump/etjump/pull/731)
* flamethrower appeared to fire when trying to fire while noclipping [#721](https://github.com/etjump/etjump/pull/721)
* gun was incorrectly positioned when leaning with non-zero roll angles [#625](https://github.com/etjump/etjump/pull/625)
* player icons could draw outside the boundaries of command map [#727](https://github.com/etjump/etjump/pull/727)
* potential crash related to map statistics and tokens on shutdown [#653](https://github.com/etjump/etjump/pull/653)
* `call/goto` print logic was inconsistent when checking for conditions for the commands [#799](https://github.com/etjump/etjump/pull/799)
* mounted MG42s could not be repaired if `g_engineerChargeTime` was set to a low value [#754](https://github.com/etjump/etjump/pull/754)
* corrected pluralization of some admin command outputs [#826](https://github.com/etjump/etjump/pull/826/files)
* "Make fireteam private?" prompt could be visible on screen after fireteam was already disbanded [#670](https://github.com/etjump/etjump/pull/670)
* `etj_drawLeaves` was trying to remap a non-existing shader [#745](https://github.com/etjump/etjump/pull/745)
* weapon heat bar did not disappear when leaving a tank while noclipping [#755](https://github.com/etjump/etjump/pull/755)
* tank MG rotation was incorrect while using `cg_thirdperson` [#756](https://github.com/etjump/etjump/pull/756)
* incorrect wording in max tracker indices error message [#786](https://github.com/etjump/etjump/pull/786)

# ETJump 2.4.0

## Breaking
* renamed confusingly named "ghostPlayers" cvars
  * `etj_ghostPlayersAlt` -> `etj_drawSimplePlayers`
  * `etj_ghostPlayersColor` -> `etj_simplePlayersColor`
  * `etj_ghostPlayerOpacity` -> `etj_playerOpacity`
  * `etj_ghostPlayerFadeRange` -> `etj_hideFadeRange`

## Added
* `etj_drawSnapHUD` cvar to draw velocity snapping HUD
* `etj_drawCGaz 5` - equivalent of DeFRaG's `df_hud_cgaz` **13**
  * `etj_CGaz5Color1-4` to set colors of no accel zone, min angle, accel zone and max angle, respectively
  * `etj_CGaz5Fov` to set FOV (max **180**, **0** uses current FOV)
* `etj_autoDemo` - automatically start demo recording on spawn/`load`, stop on timerun start
  * temporary demos recorded with a circular rotation to `demos/temp/temp_01-20`
  * `ad_save <demoname>` saves currently recorded demo manually
* `etj_gunSway` cvar to toggle gun idle/movement sway and landing bobbing
* diff times in `records <runname>` output
* `etj_speedColorUsesAccel` to color `etj_drawSpeed2` based on accel/decel
* `g_spectatorVote` cvar to allow spectators to cast and call votes
  * only voting spectators are count towards `vote_percent`, rest are ignored
* `await` command - same as `wait`, except works on background without interrupting input
  * not available during timeruns, and command queue is cleared on timerun starts
* `etj_chatScale` cvar to control chat size
* `spawnflag 4` for `target/trigger_push` to make Z velocity additive
* CHS info **53** to display Z angle of a plane
* `etj_CHSUseFeet` cvar to shift CHS Z coordinate to player feet level for position calculations
* `clearsaves` command to remove your own saves
* `etj_extraTrace` cvar to toggle tracing of playerclips on various detectors
  * `extraTrace` console command prints valid bitflag values
* center print on timerun start if `pmove_fixed` is not enabled
* playercounts for playing/spectating in scoreboard 3 headers
* `vote_allow_randommap` to control random map voting
* `g_enableVote` cvar to toggle voting on server
* `spawnflag 1` for `target/trigger_savereset` to disable save reset print
* adrenaline syringe for medics
* `deformVertexes text` shader directive support
* `cg_drawweaponIconFlash` for ETPro-style weapon icon flash coloring

## Changed
* `etj_drawCGaz 2` no longer shows the side "wings" when movement speed is < 352ups
* new color parsing system now works with crosshair colors
* new nosave, noprone and timerun icons
* players' active projectiles and mines are now removed on timerun start
* `cg_centertime` is no longer cheat protected
* reorganized settings menu
* in-game vote -> misc menu replaced with map restart & random map buttons
* `callvote map` lists matched maps if more than one map was matched
* `etj_popupGrouped` no longer groups console print unless set to **2**
* `etj_altScoreboard` default changed to **0**
* dying during timeruns instantly puts players to limbo, without creating a corpse or dropping a gun
* crosshair no longer draws when scoreboard is up
* adjusted rendering order of crosshair & crosshair names
* spectator messages (`cg_descriptiveText`) are now hidden when in-game menu is visible
* OB watcher no longer shows OBs if you are below your saved Z coordinate and have negative Z velocity
* `tracker_set` and `tracker_print` now print a warning if tracker debugging is not enabled
* text on various HUD elements no longer completely disappear when scaled to really low sizes

## Removed
* `etj_drawCGazUsers` cvar, no longer used
* several unused `vote_allow_*` cvars

## Fixed
* viewangles were not updating correctly while noclipping in prone
* crash when trying to play demos recorded in ETJump 2.3.0
* `func_static` did not open doors correctly when `spawnflag 2` was used
* limbo menu and fireteam menu could sometimes permanently shift to right on widescreen resolutions
* incorrect shaders were sometimes displaying on loading screen
* `!rename` could be used to execute console commands on the renamed player
* cvar unlockers were resetting on map changes/restarts
* connection interrupted icon ignored `etj_lagometerX/Y`
* `target/trigger_push` with `spawnflag 2` were calculating push speed incorrectly
* long console prints (over 1023 characters) could break color codes in the output
* rounding error with virtual grid on widescreen resolutions, causing 2D elements to draw at slightly incorrect positions
* firing a syringe gave players more ammo for syringe
* chat background had a solid line on the right side
* skyportal FOV was incorrectly calculated on widescreen resolutions when `etj_realFov` was enabled
* `setoffset` was adding 1 unit to players Z position when teleporting
* `func_invisible_user` was playing sounds at world origin if origin brush wasn't used
* `callvote randommap` was ignoring `g_blockedMaps`
* `callvote` with no arguments was not printing help text
* setting all `vote_allow_` cvars to **0** did not correctly disable voting
* "Connection Interrupted" text was not widescreen corrected and used old, low quality font
* color parser interpreted hex colors with `00` as single channeled color
* `tracker_set` was not able to set tracker values to negative values
* `tracker_set` could not set value outside of **1 - 50** when the command was used without specifying an index
* `etj_popupGrouped` counter was affected by the color codes of the popup
* crosshair preview in menus did not update if the menu was open when crosshair was changed via console
* crash when player reached over 83884088ups speed while CGaz 2 was enabled
* prone stopped working after server had been running for ~12,5 days without a restart
* various formatting and spelling fixes on vote strings
* erroneous print about missing portalgun models

# ETJump 2.3.0

## Breaking
* `etj_keysX/Y` are now calculated from the mid point of the keyset
* removed `etj_speedSizeX/Y` and replaced them with `etj_speedSize`

## Added
* high quality font for HUD/UI
* `nojumpdelay` worldspawn key to control no jump delay behavior on the map
  * `surfaceparm monsterslicknorth` controls jump delay on a surface
* `etj_CGazColor1/2` cvars to set custom colors for `etj_drawCGaz 2`
* `etj_autoLoad` to automatically load latest saved position when joining a team
* `nofalldamage` worldspawn key to control falldamage behavior
* `noprone` worldspawn key to control prone behavior
  * `surfaceparm donotenter` is used to define no prone areas
* `spawnflag 2048` for `trigger_multiple` - activates for all clients touching the trigger, `wait` is unique for each client
* new color parsing system for ETJump cvars that expect color values, supported formats:
  * string (white, black, green etc.)
  * normalized RGB(A) (eg. 1.0 0.5 0.75 0.33)
  * true RGB(A) (eg. 255 128 191 62), used if R, G or B component is > 1
  * hex color (eg. #ff80bf, 0xff80bf)
* cvar unlockers for various cvars to bypass cheat restrictions on them
  * `etj_drawFoliage` -> `r_drawfoliage`
  * `etj_showTris` -> `r_showtris`
  * `etj_wolfFog` -> `r_wolffog`
  * `etj_zFar` -> `r_zfar`
  * `etj_viewLog` -> `viewlog` (reimplemented)
  * `etj_offsetFactor` -> `r_offsetFactor`
  * `etj_offsetUnits` -> `r_offsetUnits`
  * `etj_speeds` -> `r_speeds`
  * `etj_lightmap` -> `r_lightmap`
  * `etj_drawNotify` -> `con_drawNotify` (note that `con_notifyTime` is not cheat protected by default)
* `etj_drawLeaves` to turn off leaves from trees and bushes
  * only works for stock shaders
* `trigger_push` entity for client side predicted pushers
* `spawnflag 2` for `target/trigger_push` to add pusher's horizontal velocity for activator instead of setting it
* `spawnflag 128` for `target_startTimer` to disable save during timerun (load interrupts the run)
* support for directories in replays menu
* `etj_quickFollow` to spectate players by aiming at them and pressing `+activate`
* `etj_drawNoJumpDelay` cvar to draw no jump delay surface indicator
* `etj_drawSaveIndicator` cvar to draw icon for save areas
* `etj_drawProneIndicator` to draw icon for no prone areas
* `etj_OBX/Y` cvars to move OB detector
* `etj_touchPickupWeapons` cvar to pickup weapons by touching them
  * requires `cg_autoactivate 1` to function
* `etj_loopedSounds` cvar to toggle playback of looped sounds in maps
* `interruptRun` client command to stop an ongoing timerun
* `noise` key support for teleport entities (plays only for activator)
* `outspeed` key for teleporters to define a fixed speed at destination
* `spawnflag 16` for teleporters to apply short knockback event at destination
  * this behavior mimics how teleporters work in Quake 3
* `noise` and `volume` key support for `func_invisible_user`
* `spawnflag 1` for `target_scale_velocity` to scale base speed instead of current speed
  * `time` key controls the duration for how long the speed scaling lasts
* `target_give` entity support to give items/weapons to activator
* `tracker_not_eq` key for `target/trigger_tracker`
* new scoreboard (`etj_altScoreboard 3`)
* new cvars to customize console
  * `etj_consoleAlpha` to set console background opacity
  * `etj_consoleShader` cvar to toggle console background shader
  * `etj_consoleColor` cvar to set console background (requires `etj_consoleShader 0`)
* new cvars to customize CHS
  * `etj_drawCHS2 2` to align CHS2 text to the right side of screen
  * `etj_CHS2PosX/Y` to offset CHS2 position
  * `etj_CHSShadow` to enable text shadow for CHS
  * `etj_CHSAlpha` to set CHS alpha
  * `etj_CHSColor` to change CHS color
* new cvars to customize chat
  * `etj_chatShadow` cvar to enable chat text shadows
  * `etj_chatAlpha` cvar to set chat transparency
  * `etj_chatLineWidth` to control chat width before automatic line break (in characters, **1 - 200**)
* new cvars to customize fireteam overlay
  * `etj_HUD_fireteam` to toggle fireteam overlay
  * `etj_fireteamPosX/Y` to offset fireteam overlay position
  * `etj_fireteamAlpha` to set fireteam hud alpha
* new cvars to customize popups
  * `etj_popupShadow` cvar to set text shadows for popup messages
  * `etj_popupAlpha` cvar to set popup message alpha
  * `etj_HUD_popup` cvar to enable/disable popup messages
    * value **2** draws on right side of the screen
  * `etj_popupPosX/Y` cvars to offset popup message position
* 4 new keyset themes for `etj_drawKeys`
* `etj_keysShadow` to draw shadow for keysets
* `etj_runtimerInactiveColor` to set run timer color when timerun is not active
* `etj_onRunStart/End` cvars to execute commands on timerun start/end
  * timerun interrupts counts as run end
* `etj_lagometerX/Y` cvars to offset lagometer position
* `etj_obWatcherSize` cvar to adjust OB watcher size
* `etj_obWatcherColor` cvar to set OB watcher color
* `etj_uphillSteps` to enable step sounds on very low landing speeds
* triggerable shader support (`targetShaderName/targetShaderNewName` entity keys) - execute shader remaps when entities are fired
  * more info: http://robotrenegade.com/q3map2/docs/shader_manual/triggerable-shader-entities.html
* new icon for timerun popup prints
* minimize button in game window (Windows only)
* `g_moverScale` cvar and `!moverScale` admin command to scale mover speed (**0.1 - 5.0**)
  * admin flag `v` controls access
* `g_debugTrackers` cvar to enable tracker debugging
  * grants access to `tracker_set` and `tracker_print` commands
  * timerun records are not saved when debugging is enabled
* `g_debugTimeruns` to enable timerun debugging
  * disables all run start condition checks and actions
  * cheats are enabled (noclip, god)
  * timerun records are not saved when debugging is enabled
* `setoffset` command to offset player's position (max **4096** per direction)
* `freecam` and `noclip` support in demo playback
* `b_demo_lookat` cvar for demo playback, to viewlock on specific player/entity
  * name shared with ETPro-equivalent cvar for Camtrace3D compatibility
* `incrementVar` console command - same as `cycle` but accepts decimal values
* majority of ETJump cvars are available in in-game menus
* ETJump menu included in the main menu
* warning message about serverlist exploit in server browser
* ETJump mod filter in server browser
* `strictsaveload` worldspawn key to restrict save/load while moving or dead
* `setplayerspawn` mapscript action to set activator's spawn point
* `setplayerautospawn` mapscript action to set activator's autospawn point
* `damageplayer` mapscript action to inflict damage to the activator
* `killplayer` mapscript action to kill the activator

## Changed
* `save` now includes player stance (stand/crouch/prone), and `load` restores the stance that the player was at saving
  * loading to prone additionally always forces viewangles, regardless of `etj_loadviewangles` cvar value
* snipers and engineer rifles are now valid weapons for all classes, to grant access to 100% accurate weapons for any class
  * only engineers get access to rifle grenades
* clients are no longer allowed to adjust roll angles
* fireteam and private messages are no longer logged
* `etj_hideMe` now hides the player better
  * footsteps, gun firing, pickup, reload and impact sounds are not heard by others
  * command map/compass doesn't draw player icon for others
  * bullet tracers, brass and wallmarks are not drawn for others
* serverlist no longer automatically refreshes to protect against server list redirection exploit
* timeruns no longer start if player is noclipping/has godmode enabled
* timeruns can no longer be started with non-zero roll angles
* timeruns are now interrupted if the player's ping is over **400**
* replays menu sorting is case insensitive
* `noclip` is now always allowed on non-dedicated servers running with cheats
* demos recorded with `etj_hideMe` enabled now show yourself on freecam/3rd person
* snipers and engineer rifles are now oriented "standing up"
* millisecond separator on timerun timer changed from `:` to `.`
* reworked OB Watcher to calculate overbounces without aiming at the saved location
* `cg_teamChatHeight` max value increased to **64**
* `etj_slickX` default value changed from **315** to **304**, was overlapping with OB detector
* `etj_numPopups 0` hides popups completely
* server browser trims leading whitespace from server names
* tweaked formatting for `noclip` related prints

## Fixed
* `func_static` with `spawnflag 2` would crash the server when trying activating entities that require an activator 
* `spawnflag 8` was not working on `target_speaker`
* `etj_noActivateLean` broken on player respawn
* `nooverbounce` was not preventing sticky overbounces
* portal teleports could be executed on the same frame as timeruns started
* connecting to a server with name containing specific extended ASCII characters would cause random mouse movements for other players
* remapped shaders were not reset back to their original values on map changes
* redirect exploit when serverlist was loaded
* footsteps were not correctly playing at low landing speeds
* runtimer did not reset on map restarts
* runtimer was not visible in demo playback
* timerun prints were calculating diff times incorrectly when run length was over 1 minute
* medic health regeneration was slower when `pmove_fixed` was enabled
* disallowed timerun start conditions were printing when player was already timerunning
* `time` option on `!ban` was not working
* `vote_minVoteDuration` could not be used to set minimum duration to less than 5 seconds
* `spawnflag 2` was not working on snipers and engineer rifles
* setting `g_banner1-5` cvars empty would crash the server on map load
* `etj_weaponsound 0` did not disable ammo and flamethrower sounds
* noclipping as spectator was not working correctly
* mouse cursor was invisible in menu text fields
* using `!deletelevel` to remove your current admin level and using admin commands afterwards crashed the server
* loading tags from a file was not working correctly for models
* `!ban` help text had incorrect time unit for `time` argument
* `movie_changeFovBasedOnSpeed` was not working correctly
* `!editlevel` was not creating a new level if the level you were trying to edit did not exist, as the command help claimed
* `!edituser` was printing redundant output when using `-clear` option

# ETJump 2.2.0

## Breaking
* renamed target_tracker `tracker_setIf` key to `tracker_set_if` to match the other keys

## Added
* new spawnflags for `target_startTimer`
  * `spawnflag 8` - reset the run/prevent the run from starting with `pmove_fixed 0`
  * `spawnflag 16` - disable use of save slots and backups
  * `spawnflag 32` - disable explosive weapon pickups
  * `spawnflag 64` - disable portalgun pickups
* `spawnflag 2` for `weapon_portalgun` entity to make it rotating around its center
* `spawnflag 4` for all weapon and item entities to make them bob up and down (`spawnflag 1` required)
* CHS info **50** to displays the last jump coordinates
* `nooverbounce` worldspawn key to control
  * `surfaceparm monsterslicksouth` controls overbounce behavior on a surface
* `spawnflag 8` for `target_teleporter` - same as `spawnflag 4` but preserves pitch value as well
* support for all spawnflags of `target_teleporter` in `trigger_teleport` as well
* re-vote support - change voted option up to 3 times within the first 10 seconds of a vote
* overbounce watcher (`etj_drawObWatcher`) - detect overbounces in saved locations
  * `ob_save`, `ob_load` and `ob_reset` commands to save, load and reset positon, respectively
* `etj_drawSpeed2` value **9** to display only tens in ETJump speedmeter (ignores hundreds & thousands)
* `etj_drawMaxSpeed` to display max speed achieved before loading a position
* `vote_minVoteDuration` to specify minimum time a vote will be active before passing

## Changed
* mod binaries are now compiled as C++ binaries
* maximum progression tracker (`target/trigger_tracker`) indices increased from **10** to **50**
* snipers and engineer rifles no longer have any spread
* decreased minimum distance between portals from **74u** to **24u**
* vote UI displays which option a player voted for
* spectators can now see yes/no counts on votes
* other players' portal 1 color is now green
* `!spec` no longer matches spectating players
* voice chats play same variation of a voice chat for all clients

## Removed
* `etj_drawspeedX/Y` & `etj_runTimerColor` cvars

## Fixed
* `shooter_rocket` was unable to hit players when player collision was disabled
* portalgun firing rate was lower for secondary portal
* `vid_restart` broke timerun timer
* `etj_noActivateLean` was not properly handled in server side physics
* triggering a `misc_landmine` crashed the client
* cursor click position was not correctly calculated for centered menus and limbo spawn point flags
* long map were not correctly flipped on loading screen in some scenarios, to prevent them from drawing off-screen
* inactivity drop was not working for drowned players

# ETJump 2.1.0

## Breaking
* `etj_logConsole` renamed to `etj_logBanner`

## Added
* widescreen support for HUD/UI
* `etj_realFov` to toggle between `vert-` and `hor+` FOV calculation
* `target/trigger_tracker` entities - track map progression and fire entities conditionally
  * replacement for `target_activate` and related entities
* `speed_limit` key for `target_startTimer` to limit maximum speed that the player can have when starting a timerun
  * default **700**
  * counts all directions, not just horizontal speed
* `etj_stretchCGaz` cvar to toggle widescreen correction on CGaz 2
* `etj_popupGrouped` cvar to group consecutive, identical popup messages
* `etj_speedShadow` to draw ETJump speedmeter with shadowed text
* `etj_noActivateLean` cvar to disable leaning with `+moveleft/right` when `+activate` is held
* `etj_enableTimeruns` cvar to toggle timeruns on client
* `etj_runTimerShadow` cvar to toggle shadowed text on runtimer
* `etj_runTimerAutohide` cvar to automatically hide runtimer when not running
* `etj_explosivesShake` to disable screen shake from explosives
* new chat message window with multi-line preview and character counter
* `spawnflag 1` for `target_remove_portals` to disable portal reset print
* `spawnflags 8/16` for `target_relay` to fire targets only while timerunning or only while not timerunning, respectively
* `target_interrupt_timerun` entity - stops active timerun without setting a record
* `target_set_health` entity - sets activators health to specific value
* `etj_ghostPlayersAlt` cvar - alternative single-colored shader to use for other players
  * `etj_ghostPlayersColor` - set ghost player's color
  * `etj_ghostPlayersOpacity` - set ghost player's opacity
  * `etj_ghostPlayersFadeRange` - control fade range for ghost players
* `etj_chatFlags` cvar to toggle team flags in chat messages
* support for map-specific autoexec configs on client (`autoexec_mapname.cfg`)
* `etj_drawTokens` cvar to toggle drawing collectible tokens
* `g_banners` server cvar to toggle banner system
* support for custom text with voice chats (e.g. `vsay 4 hi Good evening!`)

## Changed
* `cg_teamChatHeight` maximum value increased to **14**
* maximum chat message length increased to **200** characters
* `records <runname>` is now case insensitive
* spectators can no longer vote
* votes will now stay for full 30 seconds unless a majority of players on the server (includes spectators) votes for either option
* vote UI shows remaining time on vote after player has already voted
* `etj_speedY` default changed from **400** to **360**

## Removed
* random map mode and related cvars
* personal timer and related cvars
* referee system
* unused vote related cvars
* `cg_username` & `cg_adminpassword` cvars

## Fixed
* bullets and projectiles are no longer blocked by nonsolid players
* massive maps could cause random mouse movements when chat messages tried to print location that was out of bounds
* expanded map was not drawing if `cg_drawCompass` was set to **0**
* grenade sounds were not playing with `etj_HUD_weaponIcon 0`
* voice chats were not displaying timestamps with `etj_drawMessageTime`
* `!listbans` with page argument < 1 would crash the server
* chat messages ending with `^` character broke newlines
* likely crash on Linux with timerun timer
