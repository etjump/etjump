# ETJump 3.4.0

* improvements to `records` output [#1603](https://github.com/etjump/etjump/pull/1603) [#1739](https://github.com/etjump/etjump/pull/1739)
  * duplicate rank strings are no longer shown for tied times
  * `page` parameter is now correctly capped - it's no longer possible to request a page without any records on it
  * the total amount of records on a run is now displayed if the number of records is higher than the page size
* added `tracker_not_eq_any` and `tracker_not_eq_all` keys to `target/trigger_tracker` [#1614](https://github.com/etjump/etjump/pull/1614)
  * this allows mappers to pick desired behavior for "not equal" trackers - either all values or one of the values
  * old `tracker_not_eq` is now deprecated and functionality is provided by `tracker_not_eq_any`
* added ability to automatically play all demos from a directory with `demoQueue` command [#1616](https://github.com/etjump/etjump/pull/1616) 
  * `etj_demoQueueDir` cvar sets subdirectory inside `demos` to play demos from, leave empty to play from `demos` directory (default `demoqueue`)
  * commands:
    * `demoQueue start` - starts playback
    * `demoQueue stop` - stops playback (this must be issued to stop the playback, `ESC` will just go to next demo)
    * `demoQueue restart` - restart playback from the beginning
    * `demoQueue next` -  skips to next demo
    * `demoQueue previous` - skips to previous demo
    * `demoQueue goto <index>` - skips to specified demo in the queue
    * `demoQueue status` - prints status about the current demo queue
    * `demoQueue help [command]` - prints usage information or detailed help about a command
* added `target_ft_setrules` entity to set fireteam rules for the activators fireteam + supplementary worldspawn keys [#1625](https://github.com/etjump/etjump/pull/1625)
  * bypasses `noftnoghost` and new `noftsavelimit` & `noftteamjumpmode` worldspawn keys
  * `spawnflag 1` means only fireteam leader can activate the entity
  * keys:
    * `savelimit (-1 - 100 or reset)` - sets savelimit
    * `noghost 0/1` - disable/enable fireteam noghost
    * `teamjumpmode 0/1` - disable/enable teamjump mode
    * `leader_only_message` - message to print to activator if `spawnflag 1` is set and the activator is not the fireteam leader, `%s` will be substituted with the fireteam leaders name
* added `etj_logCenterPrint` cvar to log center prints to console [#1627](https://github.com/etjump/etjump/pull/1627)
  * consecutive, identical messages are not logged multiple times - same message will be logged only if it has already disappeared from screen, as controlled by `cg_centertime` cvar
  * several spammy prints (e.g. dynamite arming) and save messges are not logged
* console no longer prints a warning when trying to vsay current class as a spectator [#1648](https://github.com/etjump/etjump/pull/1648)
* spectators can now use the fireteam general vsay menu [#1649](https://github.com/etjump/etjump/pull/1649)
* fully removed location grid system from the mod - team/ft chats no longer display coordinates on chat [#1651](https://github.com/etjump/etjump/pull/1651)
* fixed Linux HWID generation leaking a socket [#1652](https://github.com/etjump/etjump/pull/1652)
* added `target_spawn_relay` entity to execute actions on player respawn [#1636](https://github.com/etjump/etjump/pull/1636)
  * passive entity that doesn't need to be explicitly targeted, fired automatically on respawns (not revives)
  * keys:
    * `team` - comma-separated list of teams that this relay fires for. If not set, fires for all teams, including spectators. Valid team names are "axis", "allies" and "spectator".
    * `scriptname` - name of the scriptblock to activate, calls the `activate` trigger, also supports `activate axis/allies`. Passes activator to mapscript.
    * `target` - targets to fire
* added `etj_onDemoPlaybackStart` cvar to execute commands on first frame of demo playback [#1617](https://github.com/etjump/etjump/pull/1617)
* fixed `target_savereset` print containing an unnecessary whitespace at the beginning of the message [#1658](https://github.com/etjump/etjump/pull/1658)
* fixed `etj_autoSpec` ignoring upmove when determining whether you or the spectated client are moving [#1662](https://github.com/etjump/etjump/pull/1662)
* improved performance of some HUD elements when spectating or playing back demos [#1663](https://github.com/etjump/etjump/pull/1663)
* static `dlight` entities are now fully client sided [#1668](https://github.com/etjump/etjump/pull/1668)
  * a dlight is considered static if it doesn't have `scriptname`, `targetname` or `spawnflags` set
  * client sided dlights allow more 16 dlights in a map (1024 to be precise)
  * renderer is still limited to max 32 dlights in a rendered scene
* fixed `target_laser` not dealing damage to client 0 [#1669](https://github.com/etjump/etjump/pull/1669)
* fixed `target_rumble` crashing the server if re-triggered again before the previous rumble sequence had ended [#1670](https://github.com/etjump/etjump/pull/1670)
* `weapon_portalgun` now supports `noise` key like other weapons & items, and properly displays cursorhint for pickup [#1671](https://github.com/etjump/etjump/pull/1671)
  * if `noise` is set, the sound overrides the default item pickup sound
* fixes to client-side coronas [#1673](https://github.com/etjump/etjump/pull/1673)
  * coronas that have no color set or are set to black are now set to white to match behavior of server-sider coronas
  * coronas with `scriptname` set are now correctly processed on the server rather than client
* fixed a crash if shutdown was executed during `etj_ad_stopDelay` period after a timerun was finished [#1380](https://github.com/etjump/etjump/pull/1380)
* drawspeed2, CGaz, snaphud & strafe quality meter are now interpolated by default [#1674](https://github.com/etjump/etjump/pull/1674)
  * interpolation can be disabled per element with `etj_HUD_noLerp` cvar (bitflag)
    * 1 = drawspeed2
    * 2 = CGaz
    * 4 = snaphud
    * 8 = strafe quality
* improvements to `viewpos` & `setviewpos` [#1675](https://github.com/etjump/etjump/pull/1675)
  * `viewpos` now prints viewangles for all axes
  * `setviewpos` now accepts either 3 or 6 arguments - omitting angles takes current viewangles as input
* fixed chat replay potentially writing a null JSON object in some scenarios [#1678](https://github.com/etjump/etjump/pull/1678)
* logs created by `g_dailyLogs` are now named as per ISO 8061 date standard [#1680](https://github.com/etjump/etjump/pull/1680)
  * admin logs now have a `-admin` suffix rather than a prefix
* fixed a long standing issue where client's backed up saves from a session would sometimes disappear when reconnecting to a server [#1681](https://github.com/etjump/etjump/pull/1681)
* fixed a potential crash when invalid vote argument was given [#1682](https://github.com/etjump/etjump/pull/1682)
* fixed a potential crash with `players` command [#1685](https://github.com/etjump/etjump/pull/1685)
* fixed unknown command `uiChatMenuOpen` console print when disconnecting from a server via in-game menu [#1691](https://github.com/etjump/etjump/pull/1691)
* added `etj_useExecQuiet` cvar to execute map/team-specific autoexec configs using `execq` command [1695](https://github.com/etjump/etjump/pull/1695)
  * `execq` is supported by ET: Legacy & ETe, and executes a config "quietly" (no info print to console)
  * bitflag cvar:
    * 1 - enable for map autoexecs
    * 2 - enable for team autoexecs
* added `etj_noMenuFlashing` cvar to disable the lightning effect on the menu background shader [#1699](https://github.com/etjump/etjump/pull/1699)
* added option to enable client side predicted portalgun teleports [#1689](https://github.com/etjump/etjump/pull/1689)
  * disabled by default due to the potential for PVS-related visual bugs
  * can be enabled via `g_portalPredict` server cvar (votable by clients)
    * voting can be disabled with `vote_allow_portalPredict`
  * worldspawn key `portalpredict` forcibly enables portal prediction for a map regardless of the cvar value
* fixed `portalteam 1` not working correctly and acting like `portalteam 2` [#1689](https://github.com/etjump/etjump/pull/1689)
* fixed `func_portaltarget` producing incorrectly sized portals when `portalsize` key was used [#1689](https://github.com/etjump/etjump/pull/1689)
* deprecated `g_portalDebug` server cvar in favor of `etj_portalDebug` client cvar [#1689](https://github.com/etjump/etjump/pull/1689)
* fixed `KEYS` menu button in `LAGOMETER` settings menu not working [#1702](https://github.com/etjump/etjump/pull/1702)
* added `fireteam countdown` command to perform a countdown in fireteam chat [#1703](https://github.com/etjump/etjump/pull/1703)
  * `fireteam countdown [seconds]`, if `seconds` isn't given, value is taken from `etj_fireteamCountdownLength` cvar value
  * length is capped to 1-10s
* added "Quick connect" window to main menu [#1701](https://github.com/etjump/etjump/pull/1701)
  * can store up to 5 servers for 1-click connecting to a server
  * servers are stored in `quickconnect.dat` file
  * can be disabled with `etj_drawQuickConnectMenu` cvar
* fixed missing centerprint when an Allied objective was auto-returned due to timeout [#1704](https://github.com/etjump/etjump/pull/1704)
* portalgun portals can no longer be overlapped in some scenarios if `portalteam` is enabled [#1705](https://github.com/etjump/etjump/pull/1705)
  * `portalteam 1` - fireteam members cannot overlap portals
  * `portalteam 2` - nobody can overlap portals
  * when joining a fireteam with `portalteam 1`, your portals are reset to prevent potential overlapping
* adjusted behavior of `etj_viewPlayerPortals` to filter out usable/unusable portals instead [#1706](https://github.com/etjump/etjump/pull/1706)
  * 0 - only draw portals you're able to use
  * 1 - draw all portals
  * a portal is considered usable in the following scenarios:
    * the portal is yours
    * `portalteam` is set to 1 and the portal is yours or from a fireteam member
    * `portalteam` is set to 2 (anyone can use anyone's portals)
* portalgun trails and debug bboxes are now correctly colored green/yellow for other's portals [#1707](https://github.com/etjump/etjump/pull/1707)
* overbounce detector now correctly shows overbounces on top of other solid players [#1710](https://github.com/etjump/etjump/pull/1710)
* added `dumpEntities` command to dump entities from the current map to a file [#1713](https://github.com/etjump/etjump/pull/1713)
  * `dumpEntities [optional name]`, if name isn't given, defaults to current map name
  * requires `developer 1`
* fixed potential crash when hitting the 16th checkpoint of a timerun and checkpoint drawing was enabled [#1717](https://github.com/etjump/etjump/pull/1717)
* fixed firing with `+attack2` not setting the players firing state correctly, causing issues such as flamethrower flames not drawing when fired [#1719](https://github.com/etjump/etjump/pull/1719)
* improvements to spectator info list [#1718](https://github.com/etjump/etjump/pull/1718)
  * number of spectators drawn can now be limited with `etj_spectatorInfoMaxClients`, -1 for unlimited (default)
    * if the number of spectators exceeds the limit, a hint will be displayed for the number of hidden spectators
  * active clients are now always drawn before inactive clients
  * spectator list can be drawn in bottom-up direction with `etj_spectatorInfoDirection 1`
* added `overbounce_players` worldspawn key to control overbounces performed on top of other clients [#1725](https://github.com/etjump/etjump/pull/1725)
  * 0 = no change, controlled by `nooverbounce` key (default)
  * 1 = overbounces from top of players are always allowed
  * 2 = overbounces from top of players are never allowed
* added `etj_hideFlamethrowerEffects` to hide the visual effects of flamethrower [#1726](https://github.com/etjump/etjump/pull/1726)
  * if enabled, hides flames, flame stream while firing and dlights
  * bitflag value
    * 1 = hide for self
    * 2 = hide for others
* added a short animation when portalgun portals are spawned [#1727](https://github.com/etjump/etjump/pull/1727)
* bundled fixed `.arena` files for certain maps with the mod pk3 [#1690](https://github.com/etjump/etjump/pull/1690)
* reworked timerun high ping interrupt to work based off command time delta rather than ping [#1741](https://github.com/etjump/etjump/pull/1741)
  * timeruns are now interrupted if the delta between consecutive user commands is 600ms

# ETJump 3.3.4

* fixed demo subdirectories not loading on replays menu on Windows [#1602](https://github.com/etjump/etjump/pull/1602)
* fixed nametag interpolation breaking non-ascii characters in the rest of the chat message [#1606](https://github.com/etjump/etjump/pull/1606)
* several fixes to flamethrower [#1607](https://github.com/etjump/etjump/pull/1607)
  * fixed not being able to set yourself on fire
  * fixed visual bug with flamechunks interacting with nonsolid players
  * fixed potential crash when a flamechunk spawned by `props_flamethrower` entity interacted with players/other entities
* fixed chat replay storing interpolated names incorrectly if the name had an escape character [#1608](https://github.com/etjump/etjump/pull/1608)
* fixed potential buffer overflow with `stylestring` key on `dlight` entities [1622](https://github.com/etjump/etjump/pull/1622)
* fixed listbox & dropdown menus changing selected entry when dragging the scrollbar and moving cursor over the list [#1623](https://github.com/etjump/etjump/pull/1623)
* fixed reversed dropdown menus not displaying background [#1626](https://github.com/etjump/etjump/pull/1626)
* reverted change done in 3.3.1 which changed the way `etj_autoLoad` works [#1635](https://github.com/etjump/etjump/pull/1635)
  * players are now initially placed on their active spawnpoint, and then teleported to their last save position, like `etj_autoLoad` worked prior to the change
  * this ensures players are not able to bypass any setup that a mapper has designed to be ran on player spawn, such as init triggers
* flipped logic in demo compatibility print for `sv_fps` detection support - prints are now only visible when using an outdated method, or if the info is not available [#1632](https://github.com/etjump/etjump/pull/1632)
* fixed flamethrower burn status persisting on respawn [#1637](https://github.com/etjump/etjump/pull/1637)
* spectator help text/follow text no longer disappears when chat window is opened [#1640](https://github.com/etjump/etjump/pull/1640)

# ETJump 3.3.3

* fixed a crash on UI keyhandling on 2.60b and ETL due to out of bounds array access in engine [#1601](https://github.com/etjump/etjump/pull/1601)

# ETJump 3.3.2

* fixed players producing water impacts when shot [#1544](https://github.com/etjump/etjump/pull/1544)
* tweaked changelog UI appearence [#1546](https://github.com/etjump/etjump/pull/1546) [#1550](https://github.com/etjump/etjump/pull/1550)
  * changelog window is now slightly smaller, with smaller font size
  * added fullscreen, faded background to improve readability
  * changelog rows now have alternating background colors to improve readability
* added missing entry for `etj_fixedCompassShader` to settings menu [#1547](https://github.com/etjump/etjump/pull/1547)
* fixed `shooter_rocket` calculating target deviation incorrectly [#1549](https://github.com/etjump/etjump/pull/1549)
* fixed wallbugging by saving while dead and loading [#1552](https://github.com/etjump/etjump/pull/1552)
* fixed long standing issue with players spawning at incorrect spawn locations if all spawnpoints of a desired spawn were occupied by a player [#1555](https://github.com/etjump/etjump/pull/1555)
* fixed `etj_autoSprint` not working reliably if clients FPS dropped below 125 with `pmove_fixed 1` [#1556](https://github.com/etjump/etjump/pull/1556)
* fixed `etj_drawSnapHUD 3` drawing snapzone borders at screen edges for zones which were off screen [#1557](https://github.com/etjump/etjump/pull/1557)
* fixed `etj_hide` & `etj_hideDistance` affecting `cg_drawCrosshairNames` draw range when fireteam collision was enabled [#1558](https://github.com/etjump/etjump/pull/1558)
  * movers health bar drawing now also respects `cg_drawCrosshairNames 0` like in VET
* fixed chat name tagging breaking on encoded chat messages if a player had `=` character in their name [#1561](https://github.com/etjump/etjump/pull/1561)
* added `toggleETJumpSettings` console command to open/close ETJump settings menu [#1562](https://github.com/etjump/etjump/pull/1562)
* `g_adminLog` is now used to log admin-related events [#1563](https://github.com/etjump/etjump/pull/1563)
  * admin commands using flags `b`, `C`, `A`, `k`, `m`, `P`, `R`, `s`, `T` and `c` are logged
  * admin chats are logged
    * these used to be logged in the regular log file, they are now only logged into admin log
  * authentication related events are logged (potential GUID/HWID spoofs, rejected connections due to a ban)
* added `spawnflags 16` to `func_button` and `func_invisible_user` to show wait time as a progress bar [#1565](https://github.com/etjump/etjump/pull/1565)
  * if set, both entities show a gradually filling progress bar indicating when they can be used again
  * slight inaccuracy with `func_button` - does not account for the travel time when the button returns to it's starting position
* fixed a minor memory leak in UI when switching changelog versions or switching active custom vote list with details panel open [#1567](https://github.com/etjump/etjump/pull/1567)
* enabled buggy `nojumpdelay` behavior in `solstice` and `stonehalls2` to fix some jumps in them which relied on a bug that was in the mod between versions 2.3.0 and 2.5.0 [#1570](https://github.com/etjump/etjump/pull/1570)
* increased the number of files that can be read from a directory to support e.g. massive demo directories [#1571](https://github.com/etjump/etjump/pull/1571)
* fixed mounted MG42 overheat event prediction [#1572](https://github.com/etjump/etjump/pull/1572)
* `entitylist` now shows the model number of brush entities [#1575](https://github.com/etjump/etjump/pull/1575)
* increased maximum number of brush entities in a map from `255` to `511` [#1577](https://github.com/etjump/etjump/pull/1577)
* fixed console prints appearing duplicated in some scenarios on listen servers [#1579](https://github.com/etjump/etjump/pull/1579)
* fixed crosshair drawing in speaker editor [#1580](https://github.com/etjump/etjump/pull/1580)
  * ETJump custom crosshairs now draw correctly
  * crosshair is no longer hidden while zooming
* added `changeskin` script action for changing skin used on an entity [#1581](https://github.com/etjump/etjump/pull/1581)
* fixed skins not working on `misc_constructiblemarker` using a `model2` key [#1582](https://github.com/etjump/etjump/pull/1582)
* added support for `cursorhint` key on `set/delete` script actions [#1584](https://github.com/etjump/etjump/pull/1584)
* fixed inconsistency in grouped popup counter formatting [#1585](https://github.com/etjump/etjump/pull/1585)
* improved error messages when overflowing certain configstring indices, mainly once which are used up by mappers [#1589](https://github.com/etjump/etjump/pull/1589)
* added new cvar parsing system for cvars expecting size or scale values [#1590](https://github.com/etjump/etjump/pull/1590)
  * any cvar that expects a size or scale can now either take one or two values
    * if a single value is given, size/scale is set uniformly
    * if two values are given, size/scale is set independently on X/Y axes, respectively
  * breaking changes
    * `etj_crosshairScaleX/Y` are removed, same functionality can now be achieved with `cg_crosshairSize X Y`
    * `etj_spectatorInfoSize` has been changed to `etj_spectatorInfoScale`
* added `spawnflag 1` to `target_delay` for per-client delay cycle [#1591](https://github.com/etjump/etjump/pull/1591)
  * when set, each client has their own private delay cycle instead of global cycle
  * re-triggering the entity before the delay is finished only resets the cycle of the activating client
* fixed timerun entity validation [#1592](https://github.com/etjump/etjump/pull/1592)
  * warnings are now printed to console if
    * map contains a start timer without a corresponding stop timer
    * map contains a stop timer without a corresponding start timer
    * map contains checkpoints for a timerun without a start and stop timer
* chat replay timestamps are now calculated on server to ensure clients system clock inaccuracies don't affect the timestamp [#1597](https://github.com/etjump/etjump/pull/1597)
* flamethrower now correctly ignores nonsolid players [#1598](https://github.com/etjump/etjump/pull/1598)
* portal gun can no longer be fired through solid players [#1599](https://github.com/etjump/etjump/pull/1599)

# ETJump 3.3.1

* fixed backstab hint showing for nonsolid players when holding a knife [#1492](https://github.com/etjump/etjump/pull/1492)
  * also fixes a long standing issue where knife would "hit" nonsolid players
* fixed `etj_drawPlayerBBox` + `etj_hideFadeRange` interaction with solid players - bbox alpha now ignores this correctly for solid players [#1493](https://github.com/etjump/etjump/pull/1493)
* added `etj_snapHUDBorderThickness` to control `etj_drawSnapHUD 3` border thickness [#1494](https://github.com/etjump/etjump/pull/1494)
  * valid range is `0.1 - 10.0 or etj_snapHUDHeight * 2`
* added `g_chatReplay` cvar to turn off chat replay on server [#1497](https://github.com/etjump/etjump/pull/1497)
  * chats are still logged when the feature is turned off, the messages are just not sent to clients
* fixed overbounce watcher breaking until cgame was reloaded if an impossible OB was detected (negative z-velocity & start height > current height) [#1496](https://github.com/etjump/etjump/pull/1496)
* added experimental `etj_smoothAngles` cvar to decouple client view updates from Pmove update rate [#1495](https://github.com/etjump/etjump/pull/1495)
  * when enabled, clients viewangles used to render the scene are updated every frame with new values, as opposed to using viewangle results from Pmove, which would limit the updates to 125Hz with `pmove_fixed 1`
  * every time client runs Pmove via prediction code, the client viewangles are synced back to match with the viewangle results from Pmove
  * does not affect physics calculations - the updates ran outside of Pmove calls are purely visual
    * this means that e.g. at 250FPS, every other viewangle update is not actually what is being sent to Pmove to calculate physics
  * no effect while spectating or if `pmove_fixed` is `0`
* fixed old etmain bug which caused bullet flesh impact sounds to play at wrong location [#1500](https://github.com/etjump/etjump/pull/1500)
* fixed corpses producing water impact particles when shot [#1501](https://github.com/etjump/etjump/pull/1501)
* fixed bullet impacts producing particle effects unreliably [#1503](https://github.com/etjump/etjump/pull/1503)
* added `cursorhint` key to `func_button` for specifying the cursorhint, similar to `func_invisible_user` [#1507](https://github.com/etjump/etjump/pull/1507)
* `etj_hideMe` can no longer be set if fireteam player collision is enabled, and will automatically be disabled if turned on [#1506](https://github.com/etjump/etjump/pull/1506)
* added `etj_ad_stopInSpec` to automatically stop autodemo recording when switching to spectators [#1510](https://github.com/etjump/etjump/pull/1510) [#1542](https://github.com/etjump/etjump/pull/1542)
* fixed `trigger_teleport_client` not working correctly for spectators [#1511](https://github.com/etjump/etjump/pull/1511)
* fixed JSON files without a JSON object unexpectedly crashing the game [#1514](https://github.com/etjump/etjump/pull/1514)
* fixed ammo packs not functioning correctly when picked up by a field ops [#1517](https://github.com/etjump/etjump/pull/1517)
* fixed `etj_autoLoad` and `load` executed during a death sequence initially spawning you in original spawn location, which could cause entities to trigger unexpectedly on original spawn location [#1516](https://github.com/etjump/etjump/pull/1516)
* fixed `target_init` and timerun start not clearing the ammo for removed weapons, which would cause the weapons to have incorrect ammo when picked up again [#1515](https://github.com/etjump/etjump/pull/1515)
* changes to chat replay [#1508](https://github.com/etjump/etjump/pull/1508) [#1521](https://github.com/etjump/etjump/pull/1521)
  * messages are now timestamped with relative timestamps (10s ago, 1min ago, 1h ago etc.)
  * `g_chatReplayMaxMessageAge` now only applies to messages that were sent prior to your session starting
    * any message send after you have connected to the server will always be replayed, regardless of the message age
    * default is now `5`
* bundled `sv_fps 125` compatible mapscript for `GreenJumps_f.bsp` with the mod [#1518](https://github.com/etjump/etjump/pull/1518)
* added `etj_autoSprint` to flip `+sprint` button behavior [#1519](https://github.com/etjump/etjump/pull/1519)
  * when enabled, player automatically sprints, and holding `+sprint` runs instead
* fixed exploits with `noghost` fireteam rule upon joining [#1520](https://github.com/etjump/etjump/pull/1520)
  * clients are now flagged to have enabled `noghost` upon joining, if `noghost` is enabled
  * timeruns are now interrupted when joining to fireteam with `noghost` enabled, if ongoing timerun doesn't allow `noghost`
* `changemodel` script action can now be used on `misc_gamemodel` entities too [#1543](https://github.com/etjump/etjump/pull/1543)
* increased slick and NJD detector ranges to maximum possible map size [#1530](https://github.com/etjump/etjump/pull/1530)
* adjusted timerun high ping interrupt to require sustained 100ms of lag before triggering [#1529](https://github.com/etjump/etjump/pull/1529)
  * this should help filter out small lag spikes that are caused by unstable connections, which would trigger timerun interrupts overly aggressively on `sv_fps + snaps 125`
* added initial support for `author` key to `.arena` files [#1528](https://github.com/etjump/etjump/pull/1528)
  * currently not used for anything
* fixed in-game `Vote -> Map` list breaking if `ui_netGameType` was not set to `2` [#1527](https://github.com/etjump/etjump/pull/1527)
* fixed nonsolid player pushing each other while riding on movers [#1526](https://github.com/etjump/etjump/pull/1526)
* added missing newline to print output when viewing run records for multiple seasons [#1525](https://github.com/etjump/etjump/pull/1525)
* reintroduced `etj_viewlog` setting to the menus for ET: Legacy clients [#1524](https://github.com/etjump/etjump/pull/1524)
  * ET: Legacy version 2.83.0 and newer can now toggle the external console while the game is running
* added support for engine extensions [#1531](https://github.com/etjump/etjump/pull/1531) [#1535](https://github.com/etjump/etjump/pull/1535)
  * currently supports two extensions
    * extended `CMD_BACKUP/MASK` on ET: Legacy clients to allow for higher ping + FPS combinations
    * window flash on ET: Legacy on chat mentions and incoming private messages (requires `etj_highlight & 2`)
* fixed buggy behavior with `TAB` and `UP/DOWNARROW` keys with expanded dropdown menus
* improved custom vote UI [#1534](https://github.com/etjump/etjump/pull/1534)
  * map list scrolling is now more responsive
  * map list scrolls back to first entry when selecting a new list
* added changelog to the menus [#1532](https://github.com/etjump/etjump/pull/1532)
* fixed a client crash if server had a custom vote list with an empty `name` or `callvote_text` field [#1536](https://github.com/etjump/etjump/pull/1536)
* fixed `etj_drawPlayerBBox & 4` drawing fireteam member bboxes of spectated player's fireteam instead of your fireteam [#1538](https://github.com/etjump/etjump/pull/1538)
* flamethrower no longer sets opposing teams members on fire [#1539](https://github.com/etjump/etjump/pull/1539)
* fixed flamethrower dps inconsistency if `sv_fps` did not align to 50ms frametimes [#1541](https://github.com/etjump/etjump/pull/1541)
* `ad_save` can no longer be used if the currently recorded demo isn't an autodemo temp demo [#1542](https://github.com/etjump/etjump/pull/1542)

# ETJump 3.3.0

* added `portalsize` key to `func_portaltarget` to allow scaling the size of the portal fired onto it (up to `512u`) [#1324](https://github.com/etjump/etjump/pull/1324)
* fixed negative values for `give health` not killing players correctly and leaving them in a state where the player was not setup correctly to be dead [#1328](https://github.com/etjump/etjump/pull/1328)
* lagometer improvements [#1325](https://github.com/etjump/etjump/pull/1325) [#1331](https://github.com/etjump/etjump/pull/1331)
  * added `etj_lagometerAlpha` to control transparency of lagometer background
  * added `etj_lagometerShader` to toggle displaying the background image or solid color
  * demo playback will now display snapshot delta values as ping in demo playback (ETPro/legacy style)
  * added client/server snapshot rate display to lagometer
* fixed server side framerate dependencies, the mod should now be fully compatible with higher `sv_fps` values [#1327](https://github.com/etjump/etjump/pull/1327) [#1330](https://github.com/etjump/etjump/pull/1330) [#1407](https://github.com/etjump/etjump/pull/1407)
  * `wait` key in mapscripts simulates `sv_fps 20` timings
  * disguise stealing speed is normalized to `sv_fps 20`
  * flamethrower firing range is normalized to `sv_fps 20`
  * flamethrower damage normalized to `sv_fps 20`
  * player pushing when players are stuck in each other is normalized to `sv_fps 20`
  * projectiles entering skyboxes no longer get stuck in skyboxes due to higher trace frequency
* added support for `private` keyword to `playsound` script actions to allow playing sound only to activator [#1314](https://github.com/etjump/etjump/pull/1314)
* fixed fireteam health only displaying correctly for fireteam members that are on the same team as you are [#1320](https://github.com/etjump/etjump/pull/1320)
* improvements to token system [#1334](https://github.com/etjump/etjump/pull/1334)
  * maximum tokens per difficulty increased to `32`
  * creating a token will now place it on ground level as opposed to player origin
  * slightly reduced the size of the token bbox to better match the visual size
  * tokens can no longer be collected while noclipping
* lean angles are now interpolated on spec/demo playback to smooth out viewangle transitions [#1337](https://github.com/etjump/etjump/pull/1337)
* added chat replay system [#1335](https://github.com/etjump/etjump/pull/1335) [#1488](https://github.com/etjump/etjump/pull/1488)
  * server replays 10 latest global chat messages to clients after connecting/map change/`vid_restart`
    * chats are stored on server in `chatreplay.json`
    * timestamps or chat flags are not preserved from original messages
    * `etj_chatReplay` cvar toggles the replay on client side, any chat messages you send are still included for other players chat replays
  * server cvar `g_chatReplayMaxMessageAge <minutes>` can be set to make chats expire, any message older than specified won't be included in a chat replay
    * default value `0` means chats never expire
* fixed potential crash on host game menu when over 500 maps were installed [#1343](https://github.com/etjump/etjump/pull/1343)
* added `trigger_teleport_client` entity to enable client side predicted teleports [#1332](https://github.com/etjump/etjump/pull/1332)
  * supports same keys/spawnflags as other teleport entities, except `spawnflags 4/8`
  * no multi-target teleport support (multiple destinations with random target picking)
* fixed vote start/end resetting event handler, which caused for example limbo menu to always close when a vote was called/ended [#1346](https://github.com/etjump/etjump/pull/1346)
* added ability to toggle player collision between fireteam members [#1333](https://github.com/etjump/etjump/pull/1333)
  * `fireteam rules noghost <1|on>/<0/off>` to enable/disable, also available in fireteam rule menu
  * projectiles and bullets now collide with solid players
  * added worldspawn key `noftnoghost` to disallow toggling fireteam collision in a map
  * added `spawnflags 256` to `target/trigger_starttimer` to allow toggling fireteam collission during a timerun
    * any timeruns lacking this spawnflag will not start if fireteam collision is enabled
    * if a fireteam member is currently timerunning in a run that does not allow collision, fireteam collision cannot be turned on
* fixed being unable to turn off server-side coronas [#1352](https://github.com/etjump/etjump/pull/1352)
* added shortname system to the new command system [#1338](https://github.com/etjump/etjump/pull/1338)
  * each command now has a full name as well as short name, e.g. `--name` & `-n`
  * `--help` can also be substituted with `-h`
* fixed viewangles stuttering when riding on movers while proning or using a mortar set [#1353](https://github.com/etjump/etjump/pull/1353)
* removed all references to gametype related cvars/commands [#1341](https://github.com/etjump/etjump/pull/1341)
* fixed water impact markers not working correctly [#1355](https://github.com/etjump/etjump/pull/1355)
* fixed `etj_autoPortalBinds` triggering `weapalt -> +attack2` rebinds when spectating someone holding a portalgun [#1363](https://github.com/etjump/etjump/pull/1363)
* fireteam chat fixes [#1362](https://github.com/etjump/etjump/pull/1362)
  * fixed old etmain bug which allowed fireteam chatting while not in a fireteam
  * fixed fireteam vsays not working when fireteam members were selected with `selectbuddy`
* removed `sort 16` from player shaders, which caused them to draw over muzzleflashes [#1361](https://github.com/etjump/etjump/pull/1361)
* added `g_allowSpeclock` to let servers disable speclocking [#1358](https://github.com/etjump/etjump/pull/1358)
* added `spawnflags 32` to teleport entities which disables any z-offsetting done by the teleporter [#1360](https://github.com/etjump/etjump/pull/1360)
  * when enabled, this disables the following:
    * regular teleports no longer add +1u z offset to destination if this is set
    * relative angle teleports no longer offset destination z origin by distance between player z origin and the trigger origin
* fixed `SPIN` spawnflag not working on some weapon_ entities, spinning now works correctly in all of them [#1365](https://github.com/etjump/etjump/pull/1365)
* timerun timer now displays white for tied records and first records (no previous record) [#1367](https://github.com/etjump/etjump/pull/1367)
* added `spawnflags 4096` to `trigger_multiple` to disable activation while noclipping [#1368](https://github.com/etjump/etjump/pull/1368)
* custom vote list names can no longer be just color codes, and mapnames are forced to lowercase [#1369](https://github.com/etjump/etjump/pull/1369)
* `records <runname>` no longer returns additional partial matches if the queried run is an exact match for a run [#1371](https://github.com/etjump/etjump/pull/1371)
* added `etj_drawPlayerBBox` to draw bounding boxes of players [#1372](https://github.com/etjump/etjump/pull/1372)
  * `etj_drawPlayerBBox` - bitflag value to draw bboxes
    * `1` = draw self
    * `2` = draw others
    * `4` = draw fireteam members
  * `etj_playerBBoxBottomOnly` - bitflag to draw only bottom of bbox instead of full box
    * same flags as `etj_drawPlayerBBox`
  * `etj_playerBBoxColorSelf/Other/Fireteam` - sets the color of the bbox
    * might not work correctly if using a custom shader
  * `etj_playerBBoxShader` - shader to use for drawing
* fixed `!spectate` not triggering timerun interrupt when a timerun was set to interrupt on team change [#1375](https://github.com/etjump/etjump/pull/1375)
* improvements to fireteam teamjump mode & `target_ftrelay` [#1364](https://github.com/etjump/etjump/pull/1364)
  * `target_ftrelay` no longer fires for spectators
  * added `spawnflags 1/2/4/8/16/32`
    * `4` - fire all targets intead of one random target
    * `32` - only fire for other fireteam members
    * rest match regular `target_relay`
  * improved fireteam prints related to teamjump mode
  * added menu option to toggle fireteam teamjump mode
  * added indicator to fireteam overlay to show if teamjump mode is active
* fixed excess overheat events triggering for mounted MG42s [#1376](https://github.com/etjump/etjump/pull/1376)
  * `+attack2` now works on mounted MG42s
* fixed missing newlines on some `records` prints [#1378](https://github.com/etjump/etjump/pull/1378)
* fixed chat highlighting not working for server chat messages [#1381](https://github.com/etjump/etjump/pull/1381)
* fixed unrelated fireteam/vote messages being visible simultaneously [#1383](https://github.com/etjump/etjump/pull/1383)
* fixed portalgun portals drawing mirrored on 2.60b clients [#1388](https://github.com/etjump/etjump/pull/1388)
* fixed systemcalls working unreliably on 64-bit clients [#1396](https://github.com/etjump/etjump/pull/1396)
* timeruns no longer interrupt due to too low fps with `com_maxfps 0` [#1405](https://github.com/etjump/etjump/pull/1405)
* added dropdown menus to UI for more convenient multi-selection menu entries [#1406](https://github.com/etjump/etjump/pull/1406)
* added autospec feature to automatically follow next client when idling in free spec [#1382](https://github.com/etjump/etjump/pull/1382)
  * `etj_autoSpec` - toggle on/off
  * `etj_autoSpecDelay` - time in milliseconds to wait before automatically following next client
* fixed `!listbans` output being affected by the color of "banned by" entry [#1419](https://github.com/etjump/etjump/pull/1419)
* fixed `!ban` failing if time was 0, making it impossible to ban someone permantently while also giving a reason for the ban [#1418](https://github.com/etjump/etjump/pull/1418)
* menu entries with `Custom` value now display the actual cvar value [#1416](https://github.com/etjump/etjump/pull/1416)
* menu options that are incompatible with players client are now hidden [#1413](https://github.com/etjump/etjump/pull/1413)
* added adminchat functionality [#1415](https://github.com/etjump/etjump/pull/1415)
  * can be toggled on server with `g_adminChat`
  * available to players with adminflag `S`
  * `say_admin`, `ma` and `enc_say_admin` will send a message to adminchat
  * also available as a target in regular chat window (`adminChat` command opens the message window with adminchat selected
* IP address is now visible in `!userinfo` output if a player is connected [#1417](https://github.com/etjump/etjump/pull/1417)
* dragging a scrollbar in listbox menu no longer stops the scroll if cursor moves outside the window [#1424](https://github.com/etjump/etjump/pull/1424)
* fixed `Write Config` button in settings menu not focusing on the text field upon opening [#1425](https://github.com/etjump/etjump/pull/1425)
* added `etj_drawSnapHUD 3` to draw snaphud with borders only instead of solid colored blocks [#1430](https://github.com/etjump/etjump/pull/1430)
* fixed collision on `func_fakebrush` with `CONTENTS_PLAYECLIP` not working correctly [#1429](https://github.com/etjump/etjump/pull/1429)
* re-fixed demo list sorting to be case-insensitive - this was already the case on earlier versions but broke at some point [#1437](https://github.com/etjump/etjump/pull/1437)
* added color picker to UI [#1422](https://github.com/etjump/etjump/pull/1422)
  * avaialble via `Color picker...` menu option in dropdown menus
  * sliders for RGB, HSV and Alpha, preview boxes for old/new color
    * RGB sliders can be set to either `0-1` or `0-255` range
  * interactive HSV color picker, usable with mouse
    * `MOUSE2` only adjusts saturation
    * `MOUSE3` only adjusts value
* `Vote -> Map` menu now lists all maps on the server, regardless of the amount of pk3 files on the server [#1431](https://github.com/etjump/etjump/pull/1431)
  * the list is also now sorted alphabetically
* fixes to `Vote -> Map -> Details` menu [#1442](https://github.com/etjump/etjump/pull/1442)
  * details now update automatically as you select new map on the vote list
  * levelshot is now displayed in correct aspect ratio
  * added text scrolling to the briefing drawing if it didn't fit on screen fully
  * unfortunately you'll likely never see this panel properly because the files won't be loaded with `sv_pure 1` if server has a lot of maps
* fixed `!add-customvote` deleting existing customvote file if the file contained a syntax error [#1443](https://github.com/etjump/etjump/pull/1443)
* fixed menu loading fallback not working if custom menufile was not found [#1446](https://github.com/etjump/etjump/pull/1446)
* added custom votes to vote UI [#1447](https://github.com/etjump/etjump/pull/1447)
  * details panel contains the list of maps on the server and any unavailable maps
  * menu contains a toggle for voting a random map or RTV from given list
* improvements to menu sliders [#1455](https://github.com/etjump/etjump/pull/1455) [#1466](https://github.com/etjump/etjump/pull/1466)
  * sliders no longer send cvar updates every frame mouse is not moved
  * added "cached" menu sliders, which update the real cvar value only when mouse click state changes
    * used for `etj_noclipScale` to prevent spamming userinfo updates, and `etj_menuSensitivity` to make the slider easier to use
* improved logging for tokens, motd and custom votes [#1463](https://github.com/etjump/etjump/pull/1463)
  * fixed several crashes related to JSON parsing
  * various errors from `!tokens` no longer crash the server, but are gracefully handled with an error message
* fixed `!tokens move` not shifting token to ground level like `!tokens add` [#1463](https://github.com/etjump/etjump/pull/1463)
* fixed `listinfo/customvotes` output if server has no custom votes set [#1463](https://github.com/etjump/etjump/pull/1463)
* `shooter_rocket/grenade/mortar` no longer spawn entities if entity limit is close to full, to prevent them working as an effective DoS in some maps [#1467](https://github.com/etjump/etjump/pull/1467)
* runtimer now uses correct timestamps for spectators/demo playback if server runs/was running at `sv_fps 125` [#1468](https://github.com/etjump/etjump/pull/1468)
* added info print to the start of demo playback [#1470](https://github.com/etjump/etjump/pull/1470)
  * displays mod version for demo, player name, map, server and any compatibility flags used during demo playback
* deprecated `g_debugTimeruns` as it serves no real purpose anymore [#1469](https://github.com/etjump/etjump/pull/1469)
* fixed various backwards compatibility issues with old demos [1472](https://github.com/etjump/etjump/pull/1472)
  * events are now correctly adjusted to account for new events/entity types added in ETJump 2.0.6, 2.3.0 and 3.3.0 (for this release)
* added cvars to control demo recording status line [#1475](https://github.com/etjump/etjump/pull/1475)
  * `etj_drawRecordingStatus` - toggle demo recording status on/off
  * `etj_recordingStatusX/Y` - X/Y position
  * etmain's `cg_recording_statusline` is removed in favor of these
  * the default position is very slightly shifted to left
* added `savepos/loadpos` system [#1456](https://github.com/etjump/etjump/pull/1456)
  * allows saving players position, angles, velocity, stance and timerun state into a file
    * files are saved into `etjump/savepos/<name>.dat` (if no name is given, `default.dat`)
    * can also be used in demo playback
    * timerun state can only be saved from demos recorded in ETJump 3.3.0 and newer
  * `loadpos` restores the state from a given file
  * `savepos` can be used any time, whereas `loadpos` requires cheats to be enabled
  * usage:
    * `savepos <optional name> <optional flags>` - saves a position
      * `1` - don't save velocity (will be cleared)
      * `2` - don't save pitch angle (will reset to 0)
      * if only one parameter is given, it's treated as a flag if it's numeric and one character only
    * `loadpos <optional name>` - loads a position
    * `listsavepos` - list all saved savepos files
    * `readsavepos` - reload savepos files
* `set` script action can no longer change entitys `classname` outside of `spawn` script events [#1480](https://github.com/etjump/etjump/pull/1480)
  * `classname_nospawn` can still be used as it doesn't re-spawn the entity
* added `delete` script action for deleting entities by their entity key/value pairs [#1481](https://github.com/etjump/etjump/pull/1481)
  * `delete { origin "16 16 16" }`, `delete { origin "16 16 16" targetname "foo" ... }`
  * if multiple key/value pairs are provided, an entity must match all of them to be deleted
  * not all entity keys are supported with this script action - this will be improved in future releases
    * for the currently supported keys, see [here](https://github.com/etjump/etjump/blob/d75422cb0d81be51575dffa5121079547a68a867/src/game/g_spawn.cpp#L84-L175)
* fixed `!rename` ignoring spaces for the new name [#1485](https://github.com/etjump/etjump/pull/1485)
  * the command also won't execute if the resulting name would be too long, while previously the new name was truncated to stay within limits
* `cg_fov` can now be set outside of 90-160 range in demo playback without setting `developer 1` [#1489](https://github.com/etjump/etjump/pull/1489)

# ETJump 3.2.2

* fixed CGaz 2 velocity direction lines flickering on spec/demo playback when playing at > 125FPS [#1307](https://github.com/etjump/etjump/pull/1307)
* fixed `etj_CGaz1DrawSnap` flickering on GCC builds due to fast math optimizations [#1316](https://github.com/etjump/etjump/pull/1316)
* fixed being unable to playback demos from the root of `demos` folder on ET: Legacy clients on Linux [#1310](https://github.com/etjump/etjump/pull/1310)
* added warning messages to client if a model required during runtime cannot be loaded [#1313](https://github.com/etjump/etjump/pull/1313)
* fixed coronas getting culled from view too early with `etj_realFov 1` and > 4:3 aspect ratio [#1311](https://github.com/etjump/etjump/pull/1311)
* fixed out of bounds access in tag loader causing a crash [#1317](https://github.com/etjump/etjump/pull/1317)
* fixed `etj_autoPortalBinds` not resetting bindings when switching to spectators to restore `followprev` behavior for weapalt [#1318](https://github.com/etjump/etjump/pull/1318)
* fixed being able to noclip inside `nonoclip` zones if player was crouched/prone with no room to stand up [#1323](https://github.com/etjump/etjump/pull/1323)

# ETJump 3.2.1

* fixed server crash when Auto RTV was called [#1293](https://github.com/etjump/etjump/pull/1293)
* fixed server crash with `cv rtv <list>` if a list had less valid maps that `g_rtvMapCount` [#1294](https://github.com/etjump/etjump/pull/1294)
* fixed issues with speedmeter and accel meter [#1296](https://github.com/etjump/etjump/pull/1296)
  * fixed speedmeter showing 0ups sometimes when landing on spec/demo playback
  * fixed accel speed flickering on spec/demoplayback
  * fixed advanced accel not turning back on automatically after joining a team from spec
* fixed footstep cycle getting desynced in some scenarios [#1297](https://github.com/etjump/etjump/pull/1297)
* fixed typo in `com_hunkmegs` menu tooltip [#1298](https://github.com/etjump/etjump/pull/1298)
* fixed flickering issues with `etj_CGaz1DrawSnapZone` [#1300](https://github.com/etjump/etjump/pull/1300) [#1301](https://github.com/etjump/etjump/pull/1301)
* fixed `etj_CGaz1DrawSnapZone` not respecting `etj_CGazTrueness` [#1302](https://github.com/etjump/etjump/pull/1302)
* added `etj_CGaz2WishDirUniformLength` & `etj_CGaz2WishDirFixedSpeed` cvars [#1303](https://github.com/etjump/etjump/pull/1303)
  * `etj_CGaz2WishDirUniformLength` - draws CGaz 2 wishdir line on consistent length regardless of wishdir  direction
  * `etj_CGaz2WishDirFixedSpeed` - draw wishdir line based off speed rather than movespeed values, similar to `etj_CGaz2FixedSpeed`
  * `etj_CGaz2FixedSpeed` is no longer capped to 1200ups

# ETJump 3.2.0

* fixed being able to go through/into no-noclip areas with `setoffset` [#1192](https://github.com/etjump/etjump/pull/1192) [#1287](https://github.com/etjump/etjump/pull/1287)
* increased UI memory pool, fixing the mod not running on 2.60b linux [#1191](https://github.com/etjump/etjump/pull/1191)
* added `func_portaltarget` entity [#1188](https://github.com/etjump/etjump/pull/1188)
  * portals fired on this entity will always center on the face they were fired onto
* fixed noclipping spectators appearing as axis soldiers in 3rd person and freecam [#1186](https://github.com/etjump/etjump/pull/1186)
* improved entity health bar visuals - bar now has gradient color to indicate progress & small border [#1185](https://github.com/etjump/etjump/pull/1185)
* added `etj_snapHUDActiveIsPrimary` [#1182](https://github.com/etjump/etjump/pull/1182)
  * colors active snapzone with `etj_snapHUDColor1` in all scenarios, and flips the colors accordingly
* added `etj_CGaz1DrawSnapZone` [#1175](https://github.com/etjump/etjump/pull/1175) [#1268](https://github.com/etjump/etjump/pull/1268)
  * draws the remainder of the current snapzone on CGaz 1, to visualize snap zones without snaphud
  * uses `etj_CGazFov`, will not align correctly to snaphud if CGaz and snaphud use different fov
* added support for `weapon_grenadelauncher` and `weapon_grenadepineapple` entities for spawning axis and allies grenades, respectively [#1195](https://github.com/etjump/etjump/pull/1195) [#1246](https://github.com/etjump/etjump/pull/1246)
* fixed a crash when starting a timerun or activating `target_init` with `REMOVE_STARTING_WEAPONS` spawnflag while cooking a grenade, and selfkilling afterwards [#1196](https://github.com/etjump/etjump/pull/1196)
* added `func_missilepad` entity [#881](https://github.com/etjump/etjump/pull/881)
  * explodes hand & rifle grenades on impact
  * keys:
    * `scale` - scales the explosion amount for more knockback
    * `target` - targets to fire when activated
    * `noise` - sound to play when grenade explodes
    * `volume` - volume of the sound
  * spawnflags:
    * `1` - start invisible, toggle existence when activated
    * `2` - scale applies to horizontal velocity only
    * `4` - scale applies to vertical velocity only
* fixed chat mentions breaking team/fireteam chat color [#1200](https://github.com/etjump/etjump/pull/1200)
* static coronas are no longer handled on server, and are purely client sided to not take up entity slots [#1201](https://github.com/etjump/etjump/pull/1201)
* fixed `tracker_not_eq` overriding other tracker keys - targets would fire if e.g. `_eq` check had failed, but `_not_eq` passed [#1204](https://github.com/etjump/etjump/pull/1204)
* fixed speaker editor and improved it's visuals [#1203](https://github.com/etjump/etjump/pull/1203)
* added bit support for tracker entities [#1205](https://github.com/etjump/etjump/pull/1205)
  * `tracker_bit_is_set` - check if a bit is set
  * `tracker_bit_not_set` - check if a bit is not set
  * `tracker_bit_set` - set a bit
  * `tracker_bit_reset` - reset a bit
* added support for arm64 on macOS with dual architecture binaries - mod is now natively compatible with Apple Silicon chips [#1208](https://github.com/etjump/etjump/pull/1208)
* added mapscripting support for trackers [#1207](https://github.com/etjump/etjump/pull/1207)
  * syntax: `tracker [optional index] [command] [value]`
  * if index is not set, defaults to index **1**
  * available commands:
    * `inc`
    * `abort_if_less_than`
    * `abort_if_greater_than`
    * `abort_if_not_equal`
    * `abort_if_equal`
    * `bitset`
    * `bitreset`
    * `abort_if_bitset`
    * `abort_if_not_bitset`
    * `set`
* added `spawnflags 8` to `func_invisible_user` to pass in `activator` to mapscript [#1209](https://github.com/etjump/etjump/pull/1209)
* added `usetarget` mapscript command - similar to `alertentity` but carries activator data [#1209](https://github.com/etjump/etjump/pull/1209)
* improved callvote texts on some votes [#1214](https://github.com/etjump/etjump/pull/1214)
  * `cv map` -> `Change map to <mapname>`
  * `cv devmap` -> `Change map to <mapname> (cheats enabled)`
  * `cv randommap <maplist>` -> `Random map from <listname>`
* fixed a long standing bug that could rarely cause user database corruption due to writing memory that was already freed to user db [#1206](https://github.com/etjump/etjump/pull/1206)
* added support for calling Rock The Vote from a custom vote list [#1215](https://github.com/etjump/etjump/pull/1215) [#1224](https://github.com/etjump/etjump/pull/1224)
* added `wait` key to `trigger_push` and fixed multiple activations on client side, causing prediction errors when traveling through large triggers [#1217](https://github.com/etjump/etjump/pull/1217)
  * default **100ms**, valid range is **0 - 65535**
* loading to a prone position while using mortar set is no longer possible [#1221](https://github.com/etjump/etjump/pull/1221)
* `goto/call/iwant` now teleport you to the same stance as the target/caller [#1223](https://github.com/etjump/etjump/pull/1223)
* fixed issues with `!rename` command
  * `!rename` no longer works on admins that are same/higher level than you
  * `!rename` no longer reduced name change limit (unless using it on yourself)
* fixed class and skill icon mouseover hints being misaligned on limbo panel on widescreen resolutions [#1225](https://github.com/etjump/etjump/pull/1225)
* fixed crash when using `set` mapscript command to change entity classname [#1228](https://github.com/etjump/etjump/pull/1228)
  * added option to change entity classname without calling spawn function again with `classname_nospawn`
* fixed multiple client side framerate (`com_maxFPS`) dependencies [#1229](https://github.com/etjump/etjump/pull/1229)
  * pmove related (most of these are not noticeble as they only appear with `pmove_fixed 0`):
    * weapon recoil normalized to ~125fps (slightly deviates on >333fps)
    * stamina recharge is now consistent on any fps
    * weapon overheating normalized to 125fps
    * bobcycle + footsteps normalized to 125fps
    * PM_DeadMove (corpse physics) normalized to 125fps
  * visual fixes
    * screenshakes from explosions normalized to 125fps
    * kickangles (viewkick from shooting) normalized to 333fps
    * particle effect rotation (`cg_wolfparticles`) normalized to 125fps
* fixed noclip not deactivating for spectators when entering `nonoclip` area [#1231](https://github.com/etjump/etjump/pull/1231)
  * clients also now get a print when noclip gets deactivated
* noclipping while dead is no longer possible by using noclip on the same frame as dying [#1232](https://github.com/etjump/etjump/pull/1232)
* `entitylist` now displays `scriptname` field of entites and the amount of brush entities in a map [#1235](https://github.com/etjump/etjump/pull/1235) [#1275](https://github.com/etjump/etjump/pull/1275)
* reduced re-vote cooldown from 2 seconds to 1 [#1241](https://github.com/etjump/etjump/pull/1241)
* fixed OB detector ignoring `nooverbounce` on demo playback [#1243](https://github.com/etjump/etjump/pull/1243)
* fixed dlights not updating if spawned after initial map load by using `set` to change an existing entity to a dlight [#1245](https://github.com/etjump/etjump/pull/1245)
* fixed `etj_autoLoad` not working when switching between Axis <-> Allies [#1252](https://github.com/etjump/etjump/pull/1252)
* added `etj_drawAccel` to display acceleration on x/y vectors [#1183](https://github.com/etjump/etjump/pull/1183) [#1258](https://github.com/etjump/etjump/pull/1258)
  * `etj_drawAccel` - draw acceleration
  * `etj_accelX/Y` - X/Y position
  * `etj_accelSize` - size of accel meter
  * `etj_accelColor` - color for accel meter
  * `etj_accelAlpha ` - alpha for color
  * `etj_accelShadow` - toggle shadow
  * `etj_accelColorUsesAccel` - color based off acceleration
  * `etj_accelAlign` - alignement, 0 = center, 1 = left, 2 = right
  * new option `2` for both accel meter and speed meter for displaying acceleration
    * green - optimal acceleration
    * yellow - suboptimal acceleration, should switch
    * red - no forwards acceleration
    * white - no acceleration
    * not available while spectating/on demo playback due to interpolation inaccuracy
* fixed teleporters without target processing teleporter outspeed and noise [#1253](https://github.com/etjump/etjump/pull/1253)
* `map/devmap` commands now force mapname to lowercase for consistency with callvote [#1254](https://github.com/etjump/etjump/pull/1254)
* added option to draw CGaz2 velocity direction only while under wishspeed with `etj_CGaz2NoVelocityDir 2` [#1257](https://github.com/etjump/etjump/pull/1257)
* `etj_autoPortalBinds` now defaults to `1` [#1259](https://github.com/etjump/etjump/pull/1259)
* fixed div by 0 on 1st server frame [#1260](https://github.com/etjump/etjump/pull/1260)
  * mod is now aware of `sv_fps` changes on server and changes are applied without reloading a map
* added `etj_muzzleFlash` to control muzzleflash drawing [#1265](https://github.com/etjump/etjump/pull/1265)
  * **0** - off
  * **1** - on
  * **2** - only draw on onther players
  * **3** - only draw on yourself
* reworked `cg_autoswitch` cvar - now acts as a bitflag value with various options [#1266](https://github.com/etjump/etjump/pull/1266) [#1269](https://github.com/etjump/etjump/pull/1269)
  * **0** - disabled
  * **1** - enabled
  * **2** - don't autoswitch unless replacing currently held weapon
  * **4** - don't autoswitch to portal gun
* fixed `HOME/END` keys breaking input of numeric fields in menus [#1267](https://github.com/etjump/etjump/pull/1267)
* added icons to replay menu for folders and up/home options [#1278](https://github.com/etjump/etjump/pull/1278)
* fixed weapons added to maps as `misc_gamemodel` drawing with a semi-transparent shader [#1282](https://github.com/etjump/etjump/pull/1281)
* fixed `target_remove_portals` noise playing for incorrect client(s) [#1286](https://github.com/etjump/etjump/pull/1286)
* overhauled to look of settings menu [#1284](https://github.com/etjump/etjump/pull/1284)
  * new layout with 7 main categories, each with some sub-categories
  * added a button to toggle background in the menus to improve visibility when changing settings
  * added a button to execute `writeconfig` directly from the settings menu
* fixed timeruns with `NoBackups` blocking usage of `load 1-3` when the slot was not a timerun save [#1289](https://github.com/etjump/etjump/pull/1289)
* fixed multiline tooltips sometimes rendering at incorrect height [#1290](https://github.com/etjump/etjump/pull/1290)
* fixed `target_init` with `KeepWeapons` or `RemoveStartingWeapons` not respecting `KeepPortalgun` spawnflag [#1291](https://github.com/etjump/etjump/pull/1291)

# ETJump 3.1.1

* fixed regression in OB detector causing it drawing too high [#1162](https://github.com/etjump/etjump/pull/1162)
* fixed being unable to activate multiple `trigger_multiple` entities simultaneously [#1163](https://github.com/etjump/etjump/pull/1163)
* auto rtv improvements [#1164](https://github.com/etjump/etjump/pull/1164) [#1174](https://github.com/etjump/etjump/pull/1174)
  * auto rtv can no longer be voted off if it's already off
  * successful auto rtv vote now displays if it was turned on or adjusted, and displays the correct time until next vote
  * auto rtv no longer gets blocked if `vote_allow_rtv 0` is set
  * 2 minute warning no longer prints if time gets adjusted so that there is less than 2 minutes until the next vote
* fixed double footsteps and prediction errors on `surfaceparm nodamage` [#1166](https://github.com/etjump/etjump/pull/1166)
* fixed rtv menu not drawing if client connected after rtv had already been called previously during the map, or an rtv vote was active while connecting [#1167](https://github.com/etjump/etjump/pull/1167)
* timeruns which don't reset on team change are now reset if a client goes to spec, only allies <-> axis switches don't interrupt [#1168](https://github.com/etjump/etjump/pull/1168)
* fixed (global)accum indices 8 and 9 working unreliably [#1170](https://github.com/etjump/etjump/pull/1170)
* fixed potential div by 0 in accum and playanim script functions [#1173](https://github.com/etjump/etjump/pull/1173)
* updated `g_oss` value to reflect correct macOS support - only x86_64 is supported [#1177](https://github.com/etjump/etjump/pull/1177)
* using `setoffset` now requires `/kill` like `noclip` before a timerun can be started [#1178](https://github.com/etjump/etjump/pull/1178)
* fixed potential buffer overflow in client rtv menu when `g_rtvMapCount` was set to < 9 [#1180](https://github.com/etjump/etjump/pull/1180)

# ETJump 3.1.0

* deprecated `target_printname` entity - all functionality is included in `target_print` [#1080](https://github.com/etjump/etjump/pull/1080)
  * existing `target_printname` entities continue to work normally
* added support for `wm_announce_private` script action for doing per-client announces via mapscript [#1083](https://github.com/etjump/etjump/pull/1083)
  * must be activated via entity that carries activator (e.g. `target_script_trigger` or `trigger_multiple`)
* fixed team/ft chat closing parenthesis/bracket being affected by player name colors [#1084](https://github.com/etjump/etjump/pull/1084)
* added name replacement support for `wm_announce/wm_announce_private` [#1085](https://github.com/etjump/etjump/pull/1085)
  * `%s` will be replaced with activators name, needs activation via entity that carries activator (e.g. `target_script_trigger` or `trigger_multiple`)
* fixed `cg_uinfo` bug on 64-bit Windows clients using ETe [#1086](https://github.com/etjump/etjump/pull/1086)
* fixed incorrect friction and look sensitivity in freecam [#1088](https://github.com/etjump/etjump/pull/1088)
* `players` command will now display the game engine used by each client [#1089](https://github.com/etjump/etjump/pull/1089)
  * additionally value of `snaps` is colored red, if it's less than `sv_fps` value
* OB watcher improvements [#1091](https://github.com/etjump/etjump/pull/1091)
  * `ob_save [name]` and `ob_load [name]` are now functional
  * implemented `ob_list` to display saved coordinates
  * OB watcher now correctly detects surfaces that don't allow overbounce
  * fixed coordinate prints having incorrect z height (was using `ps.origin`, now shifted to feet level)
  * most prints are now popups instead of console prints
* fixed multiple potential crashes in various places [#1093](https://github.com/etjump/etjump/pull/1093)
  * most notable ones:
    * fixed crash when firing `shooter_grenade` - the entity is now usable
    * fixed crash with `target_activate` when activated without activator
    * fixed crash with `setplayerspawn/setplayerautospawn/killplayer/damageplayer` script actions when activated without activator
* cleaned up several unused server cvars [#1111](https://github.com/etjump/etjump/pull/1111)
* fixed incorrect format specifier in `await` command causing a crash [#1120](https://github.com/etjump/etjump/pull/1120)
* `[map]` parameter in `ranks [map] [run]` command is no longer case sensitive [#1119](https://github.com/etjump/etjump/pull/1119)
* demo filenames in replay menu are now stripped of color codes [#1115](https://github.com/etjump/etjump/pull/1115)
* inactive specators are now drawn semi transparent in spectator list [#1116](https://github.com/etjump/etjump/pull/1116)
* fixed portalgun bypassing checks for `g_weapons` and `g_knifeOnly` [#1121](https://github.com/etjump/etjump/pull/1121)
* optimized etjump renderable system, potentially increasing performance in some scenarios by ~30% [#1109](https://github.com/etjump/etjump/pull/1109)
* added `etjump_game_manager` entity [#1123](https://github.com/etjump/etjump/pull/1123)
  * automatically placed on a map if no `script_multiplayer` or entities with `scriptname` are present to always allow mapscripting on any map
    * access is provided via `etjump_manager` script block
  * not meant to be placed by mappers - this functions the same as `script_multiplayer` entity
* added `etj_autoPortalBinds` cvar to automatically bind `weapalt` to `+attack2` and back when switchig to/from portalgun [#1125](https://github.com/etjump/etjump/pull/1125)
  * this effectively allows you to use portalgun with `weapalt` without having to separetly bind `+attack2`
* fixed not being able to enter wounded state during timeruns, which broke some timeruns (`2weeks` for example) [#1126](https://github.com/etjump/etjump/pull/1126)
* improved `target_remove_portals` entity [#1129](https://github.com/etjump/etjump/pull/1129) [#1130](https://github.com/etjump/etjump/pull/1130)
  * added `spawnflags 2` to fire targets when portals are removed
  * added `noise` & `volume` keys to play a sound when portals are removed
* added `target_portal_relay` entity [#1130](https://github.com/etjump/etjump/pull/1130)
  * fires targets if activator has fired less or equal amount of portals as defined by `maxportals` key since last portal reset
* portalgun portals of spectated player are no longer displayed with "others" portal colors [#1132](https://github.com/etjump/etjump/pull/1132)
* reduced portalgun portal shader size to match the actual portal size better [#1133](https://github.com/etjump/etjump/pull/1133)
* casting multiple votes is no longer possible by reconnecting [#1134](https://github.com/etjump/etjump/pull/1134)
  * votes called by a disconnecting client are also now automatically canceled
* improved `!seasons` command output [#1118](https://github.com/etjump/etjump/pull/1118)
  * seasons are now grouped to active, upcoming and past seasons
  * `Default` season is no longer displayed
  * start and end dates should be more legible
* `etj_drawCGaz` is now a bitflag cvar and allows drawing both CGaz huds at the same time [#1137](https://github.com/etjump/etjump/pull/1137)
* added checkpoint popups that display the checkpoint timer briefly on screen when hitting a checkpoint[#1139](https://github.com/etjump/etjump/pull/1139)
  * `etj_checkpointsPopup` - toggle popup
  * `etj_checkpointsPopupX/Y` - X/Y position
  * `etj_checkpointsPopupSize` - popup size
  * `etj_checkpointsPopupShadow` - toggle shadow
  * `etj_checkpointsPopupDuration` - how long the popup stays on screen
* added Rock The Vote system [#1135](https://github.com/etjump/etjump/pull/1135)
  * random map vote with a specified amount of maps to choose from
  * `callvote rtv` or `!rtv` admin command to call rtv
  * automatic rtv mode - server will call rtv every N minutes, can be voted on/off
  * server cvars:
    * `g_autoRtv` - automatic rtv interval in minutes, 0 for off
    * `g_rtvMaxMaps` - maximum amount of maps in rtv vote (2-9)
    * `vote_minRtvDuration` - minimum vote duration for rtv (like `vote_minVoteDuration` but only for rtv)
    * `vote_allow_rtv` - toggle allowing clients to call rtv
    * `vote_allow_autoRtv` - toggle allowing clients to turn autortv on/off
* fixed client vote info (such as re-vote counts) not resetting if a vote was canceled or failed [#1142](https://github.com/etjump/etjump/pull/1142)
* deprecated `trigger_multiple` spawnflags `512/2048` - all triggers are now multi-activator by default [#1144](https://github.com/etjump/etjump/pull/1144)
* fixed `backup` queue breaking on initial save to a specific slot [#1145](https://github.com/etjump/etjump/pull/1145)
* fixed `/ranks [mapname] [runname]` failing if given map name had a partial match to another map [#1148](https://github.com/etjump/etjump/pull/1148)
  * the command now also supports partial matching for the map name like it does for run name
* jump speeds are now rounded instead of truncated, like every other speed display [#1150](https://github.com/etjump/etjump/pull/1150)
* saves are no longer explicitly cleared on timerun start, only saves which were made during a timerun [#1149](https://github.com/etjump/etjump/pull/1149)
  * any save slot that was saved into during a timerun will be reset
  * backups are cleared of any backups made from timerun saves, and valid slots are pushed to the front of the queue
* timeruns no longer start if client has set `pmove_fixed 0` after spawning [#1156](https://github.com/etjump/etjump/pull/1156)
  * does not apply to timeruns which do not force `pmove_fixed 1`
* added `nowallbug` worldspawn key to prevent acceleration while stuck in a wall [#1155](https://github.com/etjump/etjump/pull/1155)
* added custom vote editing via admin commands [#1152](https://github.com/etjump/etjump/pull/1152)
  * `!add-customvote --name --full-name --maps` - adds a custom vote list
  * `!delete-customvote --name` - deletes a custom vote list
  * `!edit-customvote --list --name --full-name --add-maps --remove-maps` - edits a custom vote list
  * access is provided with admin flag `c`
  * added `/readCustomvotes` command to read customvote file
  * added `customvotes` alias for `listinfo`
* added support for no-noclip areas with `surfaceparm donotenterlarge` [#1157](https://github.com/etjump/etjump/pull/1157)
  * `etj_drawNoclipIndicator` - draw noclip area indicator, 1 = always, 2 = outside of volume, 3 = inside of volume
  * `etj_noclipIndicatorX/Y` - X/Y position
* added `etj_fixedCushionSteps` to play proper fall step sounds on cushion surfaces [#1136](https://github.com/etjump/etjump/pull/1136)
  
# ETJump 3.0.1

* fixed missing parenthesis around diff prints on seasonal record banner prints [#1057](https://github.com/etjump/etjump/pull/1057)
* `!delete-season` command is no longer case sensitive [#1058](https://github.com/etjump/etjump/pull/1058)
* fixed `--page-size` argument on `records` not working in some scenarios [#1059](https://github.com/etjump/etjump/pull/1059)
* fixed popups not printing for all other players when a player makes a new PB time [#1061](https://github.com/etjump/etjump/pull/1061)
* `listinfo` no longer prints any text regarding missing maps if no maps from the list are missing [#1062](https://github.com/etjump/etjump/pull/1062)
* `records` no longer prints records from other maps when the current map partially matches other map names on the server [#1063](https://github.com/etjump/etjump/pull/1063)
* runtimer and checkpoint timer now use real 8ms interval timer for players [#1069](https://github.com/etjump/etjump/pull/1069)
  * spectators/demo playback still display "fake" timer via cgame timer instead of command time
* fixed relative checkpoint timer (`etj_checkpointsStyle 0`) being 1ms off when player had no record on a run [#1066](https://github.com/etjump/etjump/pull/1066)
* various fixes to run/checkpoint timer [#1071](https://github.com/etjump/etjump/pull/1071)
  * checkpoint timer no longer breaks when a player hits the max checkpoint limit (16)
  * fixed checkpoint times not fading together with runtimer
  * fixed checkpoint timer position when using non-default size
  * adjusted the transition range in runtimer when closing in on PB - the transition duration is no longer hardcoded to several steps, instead transition starts at the last 10% of the run (capped to max 10s)
* added `rankings <season>` command shorthand [#1073](https://github.com/etjump/etjump/pull/1073)
* added `etj_CGaz2NoVelocityDir` cvar to hide velocity direction line on CGaz 2 [#1074](https://github.com/etjump/etjump/pull/1074)
* fixed various issues with record checking [#1075](https://github.com/etjump/etjump/pull/1075)
  * fixed banner prints displaying wrong diff - was displaying diff to your old time, now uses diff from new record to previous #1 record
    * popup message will still display diff to your previous record instead of previous #1 record
  * fixed banner prints not displaying if a player has no previous record set on a run, but breaks either the overall or seasonal record on their first run
  * fixed autodemo not saving demos with `etj_ad_savePBOnly 1` if a player made a new PB, but it wasn't a new overall record
* `loadcheckpoints` now supports partial name matching, and loaded checkpoints can be cleared by specifying `-1` as rank [#1077](https://github.com/etjump/etjump/pull/1077)
* users with 0 points are no longer included in `rankings` [#1078](https://github.com/etjump/etjump/pull/1078)
* fixed being unable to edit only season start or end date, not both, on a single command [#1079](https://github.com/etjump/etjump/pull/1079)
* all maps on a server are now logged to `maps.json` on game init [#1070](https://github.com/etjump/etjump/pull/1070)

# ETJump 3.0.0

* new timerun system [#995](https://github.com/etjump/etjump/pull/995) [#1014](https://github.com/etjump/etjump/pull/1014) [#1017](https://github.com/etjump/etjump/pull/1017) [#1020](https://github.com/etjump/etjump/pull/1020) [#1021](https://github.com/etjump/etjump/pull/1021) [#1043](https://github.com/etjump/etjump/pull/1043) [#1048](https://github.com/etjump/etjump/pull/1048) [#1050](https://github.com/etjump/etjump/pull/1050)
  * support for checkpoints (`target_checkpoint` & `trigger_checkpoint`) entities & related cvars for client
    * `etj_drawCheckpoints` - **1** - below runtimer, **2** - detached
    * `etj_checkpointsX/Y` - checkpoint timer location (only with `etj_drawCheckpoints 2`)
    * `etj_checkpointsStyle` - **0** - relative time, **1** - absolute time
    * `etj_checkpointsCount` - how many checkpoint times to draw (range 1-5)
    * `etj_checkpointsSize` - size of checkpoint timer
    * `etj_checkpointsShadow` - toggle shadow on checkpoint timer
    * `loadcheckpoints <run> [required] <rank>` - load checkpoints from another run for comparision - if `<rank>` isn't specified, loads #1 rank checkpoints
  * `trigger_starttimer` & `trigger_stoptimer` entities
  * `_ext` versions of all timerun entities for mapscripting purposes
  * timerun seasons - per-server seasonal records, defined by server admins with `!add-season` command
    * `!seasons` - lists all seasons (admin flag a)
    * `!add-season --name <name> [required] --start-date <YYYY-MM-DD> [required] --end-date-exclusive [YYYY-MM-DD]` (admin flag T)
    * `!edit-season --name <name> [required] --start-date <YYYY-MM-DD> --end-date [YYYY-MM-DD]` (admin flag T)
    * `!delete-season --name <name> [required]` (admin flag T)
      * deleting a season will delete all records associated with the season
  * ranking system - per-server rankings for players, points are calculated using the [DFWC EESystem](https://docs.google.com/forms/d/e/1FAIpQLSczfUVq5sPQlYcDLw9rY6MxodMBdIiM0HuPdocgksLR3NDtgw/viewform) (minus the nascar clause)
    * available via `!rankings` or `/rankings` commands (admin flag `a`)
    * `rankings --season <season> --page-size <size> --page <page>`
      * overall rankings are printed by default
  * improved records printing (`records/ranks/times/top`)
    * `records --season <season> --map <mapname> --run <runname> --page <page> --page-size <size>`
      * overall records are printed by default
    * shorthand command supports for `records <run>`, `records <map> <run>` & `records <season> <map> <run>`
      * all parameters support partial matching, no need to type out entire name
    * detailed records now show top 20 times by default, and always shows your time if not currently visible
    * records can now be fetched for any map on the server, regardless of which map is currently running
    * records command (and it's aliases) can now be executed as admin commands too (admin flag a)
  * `g_timeruns2Database` cvar to set new timerun database name on server (default `timeruns.v2.db`)
    * old timerun database will automatically be migrated to the new database format - no manual intervention required by server admins
* fireteam UI improvements [#1026](https://github.com/etjump/etjump/pull/1026) [#1030](https://github.com/etjump/etjump/pull/1030)
  * removed ranks and locations from fireteam UI
  * health display now respects `etj_fireteamAlpha` regardless of color being used
  * spectators are no longer drawn as having soldier as their class
  * fireteam savelimit can now be set via fireteam admin menu
  * savelimit `-1` is now the default (no restrictions) and `0` disables save
  * savelimit changes are now communicated to all fireteam members and cleared when a map changes
  * added UI element to display currently set fireteam savelimit
* fixed spectator list drawing while scoreboard is up [#1024](https://github.com/etjump/etjump/pull/1024)
* support for `autoexec_default` on client side for map-specific autoexec - executed if `autoexec_mapname` isn't present [#1023](https://github.com/etjump/etjump/pull/1023)
* reworked `g_blockCheatCvars` [#1016](https://github.com/etjump/etjump/pull/1016)
  * cvar now works as a bitflag value
    * **1** - force `cl_yawspeed 0` & `cl_freelook 1`
    * **2** - force `com_maxfps 25-125` when `pmove_fixed 0`
  * `m_pitch` is no longer enforced
* fixed being unable to pick up weapons after they were removed with `target_init` `REMOVE_STARTING_WEAPONS` spawnflag [#1028](https://github.com/etjump/etjump/pull/1028)
* added `etj_noPanzerAutoswitch` to disable autoswitch after firing a panzerfaust [#1029](https://github.com/etjump/etjump/pull/1029)
  * switch still occurs if completely out of ammo
* fixed shadow cvars getting out of sync after map restart [#1032](https://github.com/etjump/etjump/pull/1032)
* fixed `g_blockedMaps` matching maps on partial match - exact match is now required [#1035](https://github.com/etjump/etjump/pull/1035)
* added edge-only snaphud (`etj_drawSnapHUD 2`) [#1037](https://github.com/etjump/etjump/pull/1037)
  * edge thickness controllable with `etj_snaoHUDEdgeThickness` (range 1-128)
* limbo menu camera will now display a view from spectator spawn point if no limbo cameras are set in the map [#1040](https://github.com/etjump/etjump/pull/1040)
* fixed `EV_LOAD_TELEPORT` being processed on wrong clients, causing events related to `load` firing for wrong clients [#1041](https://github.com/etjump/etjump/pull/1041)
* added railbox drawing for following entities when `g_scriptDebug 1` is set for easier mapscripting [#1042](https://github.com/etjump/etjump/pull/1042) [#1048](https://github.com/etjump/etjump/pull/1048)
  * `trigger_multiple_ext` (green)
  * `trigger_starttimer_ext` (blue)
  * `trigger_stoptimer_ext` (blue)
  * `trigger_checkpoint_ext` (magenta)
  * `func_fakebrush` (red)
* bundled fixed mapscripts for `branx`, `branx2` and `freestyle` with the mod so they work out of box without a custom script [#1039](https://github.com/etjump/etjump/pull/1039)

# ETJump 2.6.0

* fixed timerun timer disappearing on demo playback when timerun ends [#835](https://github.com/etjump/etjump/pull/853)
* updated viewlog menu tooltip to inform viewlog usage on ET: Legacy clients [1bd5f8b](https://github.com/etjump/etjump/commit/1bd5f8b6c363ddf17df366462afe9462e07831d7)
* fixed upmove meter graph being stretched on widescreen resolutions [d04378c](https://github.com/etjump/etjump/commit/d04378c93775193c7c52d3c73ee4e816b3077d2b)
* added `etj_CGaz2FixedSpeed` cvar to allow specifying fixed speed for drawing CGaz 2 [#855](https://github.com/etjump/etjump/pull/855)
* listbox UI menus (demo, mods, mapvote etc.) are now drawn using shadowed text for better readability [#857](https://github.com/etjump/etjump/pull/857)
* fixed long names overlapping while following/being followed on scoreboard 3 [#862](https://github.com/etjump/etjump/pull/862)
* scoreboard now indicates if a server is running with cheats enabled [#863](https://github.com/etjump/etjump/pull/863)
* fixed being able to cast multiple votes by switching teams [#864](https://github.com/etjump/etjump/pull/864)
* `!most/leastplayed` output is now limited to 100 maps to avoid a msg overflow error in engine [#865](https://github.com/etjump/etjump/pull/865)
* vsay prints in console are no longer prefixed with a colon [9aa7acb](https://github.com/etjump/etjump/commit/9aa7acbd99daf5ba04f299d92a3073bf97490923)
* fixed certain weapon icons being affected by `r_picmip` [cc82a07](https://github.com/etjump/etjump/commit/cc82a073bcb12482229d0675c1a62dde795d0778)
* increased `!newmaps` max output to 50 maps [b38223a](https://github.com/etjump/etjump/commit/b38223a0bd7f12eaf45103b159ad4ef534774b25)
* unlocked `r_clear` via `etj_clear` [#856](https://github.com/etjump/etjump/pull/856)
* added `etj_jumpSpeedsMinSpeed` for setting a minimum threshold for jump speeds list [#866](https://github.com/etjump/etjump/pull/866)
  * any jump under the specified speed will be colored with `etj_jumpSpeedsSlowerColor`
  * works independently of `etj_jumpSpeedsShowDiff`
* added support for team-specific autoexecs [#867](https://github.com/etjump/etjump/pull/867)
* removed `!nogoto`, `!nosave`, `!putteam` and `!rmsaves` admin commands [#869](https://github.com/etjump/etjump/pull/869)
* fixed shadow cvars from getting unsynced from real ones on map changes with ETe/ETL [#870](https://github.com/etjump/etjump/pull/870)
* increased UI memory pool size for 64-bit clients [#875](https://github.com/etjump/etjump/pull/875)
* rephrased some vote related messages and moved some from console prints to popups [#876](https://github.com/etjump/etjump/pull/876)
* fixed players getting stuck when spawning on same spawnpoint with `g_ghostplayers 0` [#879](https://github.com/etjump/etjump/pull/879)
* fixed issues with player collision with other players and entities with `pmove_fixed 1` by making sure collision always uses valid mins/maxs [#880](https://github.com/etjump/etjump/pull/880)
* fixed visual bug in vote UI where voting yes as a spectator while following someone would show that you voted no [#882](https://github.com/etjump/etjump/pull/882)
* added `top` alias for viewing timerun records [#884](https://github.com/etjump/etjump/pull/884)
* removed charge time related cvars from serverinfo [#885](https://github.com/etjump/etjump/pull/885) [#889](https://github.com/etjump/etjump/pull/889)
* improved update accuracy of upmove meter and strafe quality meter, most notably fixing the delay introduced in 2.5.1 where initial update of strafe quality was delayed due to client waiting for a snapshot [#833](https://github.com/etjump/etjump/pull/883)
* removed mission time and spawntime from limbo menu [#892](https://github.com/etjump/etjump/pull/892)
* fixed chatbox lineheight calculation sometimes resulting in too small lineheights and overlapping text [#899](https://github.com/etjump/etjump/pull/899)
* removed unused `cg_predictItems` cvar [#900](https://github.com/etjump/etjump/pull/900)
* added `cv` alias for `callvote` [#902](https://github.com/etjump/etjump/pull/902)
* fixed players map icon warping when trying to `noclip` while mounted in a tank [#903](https://github.com/etjump/etjump/pull/903)
* fixed detailed run records being impossible to fetch if the run name has color codes [#904](https://github.com/etjump/etjump/pull/904)
* invalid arguments for `vote` command no longer default to `vote no` [#906](https://github.com/etjump/etjump/pull/906)
  * commands are no longer case sensitive and ignore color codes
  * invalid arguments now print a help text with valid vote arguments
* scoreboard inactivity icon will now correctly persist across map changes [#906](https://github.com/etjump/etjump/pull/907) [#913](https://github.com/etjump/etjump/pull/913)
* etjump credits menu will now automatically fetch the correct version for non-release builds too [#910](https://github.com/etjump/etjump/pull/910)
* icons in scoreboard will now correctly fade out with the rest of the scoreboard [#908](https://github.com/etjump/etjump/pull/908)
* restored vanilla lagometer behavior (don't draw lagometer on localhost by default) [#912](https://github.com/etjump/etjump/pull/912)
  * lagometer can be drawn always if desired by setting `cg_lagometer 2`
* fixed flamethrower spawning too many flames with `cg_drawGun 0` when repeatedly tapping `+attack` [#919](https://github.com/etjump/etjump/pull/919)
* fixed and improved several aspects of playerstate prediction [#916](https://github.com/etjump/etjump/pull/916)
  * added unlagged's optimized prediction via `etj_optimizePrediction` and fixed some issues it has with `pmove_fixed 1`
  * fixed jittery view when colliding with movers with `com_maxfps > 125`
  * fixed landing view animation not resetting when switching teams in the middle of the animation
  * fixed prediction errors not being cleared on respawn
  * fixed view transition happening when joining spectators while proning
* fixed a potential crash when activating `trigger_multiple` with a non-player entity [#920](https://github.com/etjump/etjump/pull/920)
* improvements to `etj_nofatigue` and stamina bar [#922](https://github.com/etjump/etjump/pull/922)
  * `PW_ADRENALINE` is no longer given to player every frame, improving prediction performance when `etj_optimizePrediction` is enabled
  * `etj_nofatigue` no longer makes stamina bar appear as if you have adrenaline - only using real adrenaline syringe will display the color lerping
* players now receive a once per map warning when starting a timerun if cheats are enabled [#924](https://github.com/etjump/etjump/pull/924)
* unlocked `r_flareSize` via `etj_flareSize` [#925](https://github.com/etjump/etjump/pull/925)
* fixed `fireteam invite/warn/kick/propose` console command handling [#928](https://github.com/etjump/etjump/pull/928)
  * action by clientnum now sends the command to correct client instead of the previous client
  * action by clientname now returns invalid clientnum and errors if matching multiple clients
* fixed fireteam admin menu drawing behind certain HUD elements [#936](https://github.com/etjump/etjump/pull/936)
* added `etj_menuSensitivity` for controlling mouse sensitivity in UI/menus [#931](https://github.com/etjump/etjump/pull/931)
* added `listspawnpt` command to list valid spawnpoints in a map [#933](https://github.com/etjump/etjump/pull/933)
* removed mapscript actions `cvar set/inc/random` [#934](https://github.com/etjump/etjump/pull/934)
* added `etj_footstepVolume` to control volume of movement-generated sounds (stepsounds/landings) [#938](https://github.com/etjump/etjump/pull/938)
* fixed runtimer colors (red/green when record is set) breaking when the run name had spaces [#941](https://github.com/etjump/etjump/pull/941)
* new #1 records on timeruns are now broadcasted with a banner print [#935](https://github.com/etjump/etjump/pull/935)
  * diff column coloring on `ranks` command is now reversed, meaning faster records before you are colored green with `-` as opposed to red with `+`
* portalgun portals no longer render when they're out of PVS, fixing portals sometimes being visible through skyboxes due to shader sorting order [#949](https://github.com/etjump/etjump/pull/949)
* fixed incorrect error message when `misc_gamemodel` was used with `ANIMATE` spawnflag with 0 frames set [89d7f32](https://github.com/etjump/etjump/commit/89d7f320d0b9c925be51f55379f29da72d3b2483)
* fixed player head being a valid portal surface [#957](https://github.com/etjump/etjump/pull/957)
* fixed portalgun trails not respecting `etj_viewPlayerPortals` [#959](https://github.com/etjump/etjump/pull/959)
* `load` will now respawn & load players position if executed during deathcam after falling to death [#962](https://github.com/etjump/etjump/pull/962)
* improved spectator info list [#964](https://github.com/etjump/etjump/pull/964)
  * `etj_drawSpectatorInfo` now accepts 1-3 values, 1 = left align, 2 = center align, 3 = right align
  * `etj_spectatorInfoSize` and `etj_spectatorInfoShadow` to set text size and enable shadow, respectively
  * `etj_spectatorInfoY` is now calculated from top-left rather than bottom-left of first name on the list, so value + will no longer hide the first player off screen
    * Default was changed from 40 -> 30
* added `entitylist` to console autocompletion and improved it's output [#954](https://github.com/etjump/etjump/pull/954)
* CGaz 2 is now correctly centered on crosshair instead of being slightly shifted to left [#966](https://github.com/etjump/etjump/pull/966) [#999](https://github.com/etjump/etjump/pull/999)
* fixed players not drowning consistently when using `pmove_fixed 1` and `com_maxfps > 125` [#984](https://github.com/etjump/etjump/pull/984)
* `cg_runroll` and `cg_runpitch` default values are now 0 [#987](https://github.com/etjump/etjump/pull/987)
* added `nodrop` worldspawn key to globally enable nodrop in the map [#986](https://github.com/etjump/etjump/pull/986)
  * when enabled, items will only drop to ground inside `CONTENTS_NODROP` brushes
* added a warning when starting server a on ET: Legacy or ETe without vanilla patch collision [#967](https://github.com/etjump/etjump/pull/967)
* quadrupled the number of supported maps on a server by the mod, should be roughly 2000 maps now [#990](https://github.com/etjump/etjump/pull/990)
* fixed certain indicators being inaccurate during playback at the cost of slower update rate, most notably OB detector [#992](https://github.com/etjump/etjump/pull/992)
* added missing `say_buddy` and `enc_say*` commands to tab completion and removed some invalid commands [#994](https://github.com/etjump/etjump/pull/994)
* added 7 new crosshairs drawn as vector graphics [#993](https://github.com/etjump/etjump/pull/993)
  * `etj_crosshairScaleX/Y` to scale on x/y (also works for old crosshairs, range -5-5)
  * `etj_crosshairThickness` to set line thickness (range 0-5)
  * `etj_crosshairOutline` toggles drawing outline with the new crosshairs
* improved output of `listinfo` command [#996](https://github.com/etjump/etjump/pull/996)
* fixed `tracker_not_eq` key on tracker entities working as an OR statement instead of AND - targets will now fire only if ALL specified values do not match [#997](https://github.com/etjump/etjump/pull/997)
* timeruns no longer start if a client has used `noclip` after respawning [#998](https://github.com/etjump/etjump/pull/998)

# ETJump 2.5.1

* fixed weapon getting swapped briefly to primary weapon when starting a timerun while not holding a primary weapon [#832](https://github.com/etjump/etjump/pull/832)
* fixed jump speeds, strafe quality and upmove reset sometimes not resetting on death due to dropped events [#834](https://github.com/etjump/etjump/pull/834)
* fixed view transitions happening on load when stance is different than saved stance (e.g. load from stand to prone) [#837](https://github.com/etjump/etjump/pull/837)
* fixed a crash with ETe in localhost when changing a map while playing (not speccing) [#835](https://github.com/etjump/etjump/pull/835)
* fixed `etj_speedColorUsesAccel` ignoring `etj_speedAlpha` [#840](https://github.com/etjump/etjump/pull/840)
* renamed `savelimit` worldspawn key to `limitedsaves` to avoid issues with old maps which might have set the key prior to it working in 2.5.0 [#841](https://github.com/etjump/etjump/pull/841)
* (hopefully) fixed random crashes related to pmove utils [#835](https://github.com/etjump/etjump/pull/835) [#844](https://github.com/etjump/etjump/pull/844)
* updated `g_oss` default value to __399__ to reflect Win x64/M1 Mac support [e5f6a79](https://github.com/etjump/etjump/commit/e5f6a79402c56bbe655eac6cc4ef27b533b08954)
  * the cvar is now read-only as it's not meant to be changed by server operators [#850](https://github.com/etjump/etjump/pull/850)
* fixed FPS on scoreboard displaying 0 when client had `com_maxfps` > 998 [#849](https://github.com/etjump/etjump/pull/849)
* added `etj_CGazOnTop` for drawing CGaz on top of Snaphud [#848](https://github.com/etjump/etjump/pull/848)

# ETJump 2.5.0

* added silent command flag `/` to `!listflags` [#557](https://github.com/etjump/etjump/pull/557)
  * `!help` will print a note if you have permission to execute silent comands
* fixed unintended vote logic behavior where 50/50 split votes would pass instead of fail with `vote_percent 50` [#560](https://github.com/etjump/etjump/pull/560)
* added `etj_speedAlign` to control speed meter alignment [#561](https://github.com/etjump/etjump/pull/561)
  * __0__ center (default)
  * __1__ left align
  * __2__ right align
* fixed default scoreboard header playercount spelling when team has only one player [#565](https://github.com/etjump/etjump/pull/565)
* added timerun status indicator to scoreboard [#566](https://github.com/etjump/etjump/pull/566), [#568](https://github.com/etjump/etjump/pull/568)
* added idle indicator to scoreboard [#568](https://github.com/etjump/etjump/pull/568)
* added `unload` command to revert last `load` command [#569](https://github.com/etjump/etjump/pull/569)
  * cannot be used during timeruns
  * cannot be used to teleport to an area where you cannot use `save`
  * only __1__ unload slot, overwritten on every successful `load` command
* replaced `etj_weaponSound` with `etj_weaponVolume` to allow adjusting volume of weapon sounds [#570](https://github.com/etjump/etjump/pull/570)
* fixed timerun finish prints looping back to `00` if run takes more than 1 hour [#567](https://github.com/etjump/etjump/pull/576)
* fixed issues/inconsistencies with portalgun pickup [#577](https://github.com/etjump/etjump/pull/577)
  * fixed weapon switch animation occuring when pickup up portalgun
  * added missing `Picked up a portalgun` print
  * added missing pickup sound
* reintroduced Quake 3 constantLight support [#579](https://github.com/etjump/etjump/pull/579)
  * use `light <value>` and `color <R G B>` to make brush entites emit light
* added `noise` key to `func_button` to allow using custom sounds [#580](https://github.com/etjump/etjump/pull/580)
  * use `nosound` as `noise` key to disable sound completely
* added `spawnflags` __8__ to `func_static` to allow disabling random __500-1500ms__ delay between activations [#582](https://github.com/etjump/etjump/pull/582)
  * minimum delay between activations when using this spawnflag is __100ms__
* added `etj_drawBanners` to allow hiding banners [#587](https://github.com/etjump/etjump/pull/587) [#805](https://github.com/etjump/etjump/pull/805)
* `!listmaps` now displays total number of maps on server [#588](https://github.com/etjump/etjump/pull/588)
* fixed duplicate spam protection print with `team` command [#591](https://github.com/etjump/etjump/pull/591)
* added back binoculars to field ops [#593](https://github.com/etjump/etjump/pull/593)
  * calling artillery is only possible when cheats are enabled
* added initial support for macOS (10.10 and up) [#590](https://github.com/etjump/etjump/pull/590), [#595](https://github.com/etjump/etjump/pull/595)
  * very untested and possibly broken - feedback appreciated
  * no M1 mac support
* added `target_init` entity - initalizes player to a freshly spawned state [#596](https://github.com/etjump/etjump/pull/596)
* fixed weapon swapping on timerun start not handling covert ops weapons properly [#598](https://github.com/etjump/etjump/pull/598)
* timescale is no longer reset on demo playback when subsystems are restarted (eg. calling `vid_restart`) [#599](https://github.com/etjump/etjump/pull/599)
* `etj_drawOB 2` no longer shows stickies that are on the same height as you are currently [#600](https://github.com/etjump/etjump/pull/600)
* removed sway from binoculars & added binocular-like zoom for spectators [#601](https://github.com/etjump/etjump/pull/601)
  * cgaz and snaphud no longer draw when zooming in with binoculars or scoped weapons
* timeruns will now interrupt if client sets `com_maxfps < 25` to prevent trigger abuse [#602](https://github.com/etjump/etjump/pull/602)
* added CHS info __55__ to display last jump speed [#603](https://github.com/etjump/etjump/pull/603)
* added `etj_drawJumpSpeeds` to display last 10 jump speeds [#608](https://github.com/etjump/etjump/pull/608) [#803](https://github.com/etjump/etjump/pull/803)
  * `etj_drawJumpSpeeds` - toggle on/off (default `0`)
  * `etj_jumpSpeedsX/Y` - X/Y offset (default `0/0`)
  * `etj_jumpSpeedsColor` - RGBA color (default `1.0 1.0 1.0 1.0`)
  * `etj_jumpSpeedsShadow` - toggle shadow (default `1`)
  * `etj_jumpSpeedsStyle` - (bitflag) __0__ = vertical, __1__ = horizontal, __2__ no label text, __4__ reversed list (default `0`)
  * `etj_jumpSpeedsShowDiff` - color jumps speeds based on if it was faster/slower than previous jump (default `0`)
  * `etj_jumpSpeedsFasterColor` - RGBA color for faster speeds (default `0.0 1.0 0.0 1.0`)
  * `etj_jumpSpeedsSlowerColor` - RGBA color for slower speeds (default `1.0 0.0 0.0 1.0`)
* reinforcement timer is now hidden when `cg_descriptiveText` is set to __0__ [#612](https://github.com/etjump/etjump/pull/612)
* `etj_speedSize` now accepts floating point values [#617](https://github.com/etjump/etjump/pull/617)
* `etj_CGazAlpha` no longer affects CGaz 5 [#618](https://github.com/etjump/etjump/pull/618)
* spectators can no longer use `spec(un)lock` [#619](https://github.com/etjump/etjump/pull/619)
* added missing `speclist` client command for autocompletion [#619](https://github.com/etjump/etjump/pull/619)
* fixed a case where certain NJD pad combinations allowed to bypass jump delay on standard surfaces [#622](https://github.com/etjump/etjump/pull/622)
* fixed gun position when leaning with non-zero roll angles [#625](https://github.com/etjump/etjump/pull/625)
* fixed color cvars displaying incorrectly in menus on 64-bit clients [#626](https://github.com/etjump/etjump/pull/626), [#629](https://github.com/etjump/etjump/pull/629)
* fixed CGaz showing incorrect angles on ground strafing [#635](https://github.com/etjump/etjump/pull/635)
* fixed client ID 0 sounds being played from too far on OpenAL sound backend [#644](https://github.com/etjump/etjump/pull/644)
* added strafe quality meter to display percentage of pmove frames spent on optimal accel zone [#641](https://github.com/etjump/etjump/pull/641)
  * fully accurate only while playing - spectator/demo playback is an rough approximation due to `sv_fps 20` (50ms commandTime intervals instead of 8ms)
  * `etj_drawStrafeQuality` - toggle on/off (default `0`)
  * `etj_strafeQualityX/Y` - X/Y offset (default `0/0`)
  * `etj_strafeQualityColor`- RGBA color (default `1.0 1.0 1.0 1.0`)
  * `etj_strafeQualityShadow` - toggle shadow (default `1`)
  * `etj_strafeQualitySize`- text size (default `3`)
  * `etj_strafeQualityStyle` - display style, __0__ = label + percentage, __1__ = percentage, __2__ = value only
* expanded map is now always drawn on top of HUD [#630](https://github.com/etjump/etjump/pull/630)
* increased `G_Alloc` memory pool size to avoid crashes on some maps on 64-bit clients/servers [#654](https://github.com/etjump/etjump/pull/654)
* tied timeruns records are now displayed in white rather than red [#658](https://github.com/etjump/etjump/pull/658)
* fixed potential segfault with mapstatistics and tokens on shutdown [#653](https://github.com/etjump/etjump/pull/653)
* added `g_oss` server cvar to improve integration with ETLegacy server browser [#665](https://github.com/etjump/etjump/pull/665)
* fixed "Make fireteam private?" prompt staying on screen after already disbanding fireteam [#670](https://github.com/etjump/etjump/pull/670)
* fixed `!listmaps` and `!map` commands ignoring `g_blockedMaps` [#669](https://github.com/etjump/etjump/pull/669)
* improved `etj_altScoreboard 3` [#673](https://github.com/etjump/etjump/pull/673)
  * fixed hostname and mapname overlapping - full map name is now always drawn at the expense of truncating hostname
  * made the scoreboard overall a bit bigger
* changed and added several features to CGaz and snaphud [#666](https://github.com/etjump/etjump/pull/666) [#686](https://github.com/etjump/etjump/pull/686)
  * CGaz/snaphud are no longer drawn on ladders or underwater
  * CGaz/snaphud are no longer drawn while noclipping or dead
  * CGaz/snaphud are no longer drawn while mounted or using set weapons
  * added `etj_projection` to set projection type of CGaz 1 and snaphud
    * __0__ = rectilinear projection (default, current)
    * __1__ = cylindrical projection
    * __2__ = panini projection
  * CGaz changes
    * removed all CGaz HUDs except 2 & 5
      * CGaz 5 is now CGaz 1
    * removed unused `etj_CGazWidth` and `etj_CGazAlpha` cvars
    * renamed following cvars
      * `etj_CGazColor1` -> `etj_CGaz2Color1`
      * `etj_CGazColor2` -> `etj_CGaz2Color2`
      * `etj_CGaz5Color1` -> `etj_CGaz1Color1`
      * `etj_CGaz5Color2` -> `etj_CGaz1Color2`
      * `etj_CGaz5Color3` -> `etj_CGaz1Color3`
      * `etj_CGaz5Color4` -> `etj_CGaz1Color4`
      * `etj_CGaz5Fov` -> `etj_CGazFov`
     * added `etj_CGazTrueness` - bitmask value for trueness of CGaz (default __2__)
       * __1__ = show upmove influence
       * __2__ = show true groundzones (CGaz 1 only)
      * `etj_CGaz1Color1-4` default alpha values changed from __1.0__ to __0.75__
      * `etj_CGazY` default value changed from __260__ to __240__
  * snaphud changes
    * added `etj_snapHUDHLActive` - highlight active snapzone (default off)
      * `etj_snapHUDHLColor1/2` to set colors
    * added `etj_snapHUDTrueness` - bitmask value for trueness of snaphud (default __0__)
      * __1__ = show upmove influence
      * __2__ = show true groundzones
* `g_password` is no longer stored in `ClientUserinfo` [#682](https://github.com/etjump/etjump/pull/682)
* clamped `target_scale_velocity` `BASE` scaling to __0.25__ - __3.0__ [#687](https://github.com/etjump/etjump/pull/687)
* item pickup sound is no longer tied to `etj_itemPickupText` [#692](https://github.com/etjump/etjump/pull/692)
* fixed following sounds not being affected by `etj_weaponVolume` [#694](https://github.com/etjump/etjump/pull/694)
  * reload sound
  * weapon switch sound
  * satchel detonator idle sound
  * panzer/satchel detonator spin-up
  * mounted MG42s and tanks
  * overheating sounds
  * dynamite spin-up
* fixed incorrect cvar flags for `shared` causing issues with ET:L/ETe clients [#701](https://github.com/etjump/etjump/pull/701)
* dead players can no longer use `noclip` [#714](https://github.com/etjump/etjump/pull/714)
* fixed player view transition errors when following players from free spec [#715](https://github.com/etjump/etjump/pull/715) [#717](https://github.com/etjump/etjump/pull/717)
  * view no longer transitions from crouch to prone when following prone players
  * view no longer persistently shifts after step-ups and hard landings upon following
* revive icon alpha value is no longer tied to spawntimes [#718](https://github.com/etjump/etjump/pull/718)
* fixed potential division by zero in map grid code causing crash with certain client/OS setups [#716](https://github.com/etjump/etjump/pull/716)
* fixed a visual bug which made flamethrower appear to be able to fire while noclipping [#721](https://github.com/etjump/etjump/pull/721)
* fixed `configstrings` not including `MAX_FIRETEAMS` in its value [#725](https://github.com/etjump/etjump/pull/725)
* map grid is no longer drawn on command map [#726](https://github.com/etjump/etjump/pull/726)
  * grid system is still calculated in the background for command map positions etc.
* player icons no longer draw outside the boundaries of command map [#727](https://github.com/etjump/etjump/pull/727)
* removed server sided `g_saveMsg` and added client sided `etj_saveMsg` - clients can now specify their own save message [#729](https://github.com/etjump/etjump/pull/729)
* fixed speedmeter showing 1ups while wounded on a non-flat surface [#731](https://github.com/etjump/etjump/pull/731)
* improved command map accuracy by using latest PVS data for all players [#734](https://github.com/etjump/etjump/pull/734)
* crosshair is no longer hidden while moving in prone [#744](https://github.com/etjump/etjump/pull/744)
* fixed `etj_drawLeaves` trying to remap a non-existing shader [#745](https://github.com/etjump/etjump/pull/745)
* fixed mounted MG42s being unrepairable when `g_engineerChargeTime` was set to a low value [#754](https://github.com/etjump/etjump/pull/754)
* fixed weapon heat bar not disappearing when unmounting tank while noclipping [#755](https://github.com/etjump/etjump/pull/755)
* fixed tank MG rotation being incorrect while using `cg_thirdperson` [#756](https://github.com/etjump/etjump/pull/756)
* corrected some incorrect cvars from both client and server
  * removed `shared`, `cg_synchronousClients` and `pmove_msec` cvars from cgame cvars [#762](https://github.com/etjump/etjump/pull/762)
  * removed `pmove_fixed` cvar from server, renamed `mapname` -> `sv_mapname`[#764](https://github.com/etjump/etjump/pull/764)
* added `etj_expandedMapAlpha` to control transparency of expanded map [#771](https://github.com/etjump/etjump/pull/771)
* removed `Objective:` text from loading screen and made mapname font size to scale automatically to the boundaries of the box [#770](https://github.com/etjump/etjump/pull/770)
* prevented several commands from being executed during demo playback to avoid potentially unwated behavior [#757](https://github.com/etjump/etjump/pull/757)
* added several new binding options to ETJump binds menu [#774](https://github.com/etjump/etjump/pull/774)
* `cg_skybox` is no longer cheat protected so skyportals can be turned off [#776](https://github.com/etjump/etjump/pull/776)
* fixed incorrect wording in max tracker indices error message [#786](https://github.com/etjump/etjump/pull/786)
* added a new portalgun model and portal shaders [#787](https://github.com/etjump/etjump/pull/787) [#822](https://github.com/etjump/etjump/pull/822)
  * portalgun range is increased to 131072 units [#824](https://github.com/etjump/etjump/pull/824)
* improved spacing of multiline tooltips in menus [#788](https://github.com/etjump/etjump/pull/788)
* speclock improvements:
  * speclock and specinvites are now persistent across map changes [#769](https://github.com/etjump/etjump/pull/796)
  * disconnecting will now invalidate your invitation to spectate someone, preventing other players to unintentionally bypass the lock [#789](https://github.com/etjump/etjump/pull/789)
* `cg_drawRoundTimer` will now display the elapsed time of current map [#791](https://github.com/etjump/etjump/pull/791)
* added `etj_FPSMeterUpdateInterval` to control the update rate of FPS meter[#793](https://github.com/etjump/etjump/pull/793)
* fixed binocular/scope overlay disappearing while scoreboard is up [#794](https://github.com/etjump/etjump/pull/794)
  * the reticles are now hidden while scoreboard is up to improve readability
* fixed some inconsistencies with `call/goto` print logic when checking for conditions for the commands [#799](https://github.com/etjump/etjump/pull/799)
* changed default `g_mapScriptDir` cvar value from `scripts` -> `mapscripts` for consistency between mostly all other mods [#804](https://github.com/etjump/etjump/pull/804)
* improvements to handling of broken/missing command map shaders [#800](https://github.com/etjump/etjump/pull/800)
  * added a default command map to display when a map does not include one - no more ugly black/orange texture
  * added `etj_fixedCompassShader` cvar - when enabled, will override the command map shader used by the current map with a shader that ensures proper masking of command map, preventing map corners from drawing outside of compass borders
    * the original image is still used, only the shader contents are overridden
    * this will break some special command maps when enabled such as the animated command map in Skacharohuth, but in turn will always ensure proper masking
* `g_maxConnsPerIP` can no longer be set to negative value [#807](https://github.com/etjump/etjump/pull/807)
* added upmove meter to display how long `+moveup` is being held for while jumping [#802](https://github.com/etjump/etjump/pull/802)
  * displays 4 statistics: ground/pre-jump - total - post jump (bottom to top on HUD)
  * fully accurate only while playing - spectator/demo playback is an rough approximation due to `sv_fps 20` (50ms commandTime intervals instead of 8ms)
  * customizeable with following cvars
    * `etj_drawUpmoveMeter` - (bitflag) __0__ = off (default) __1__ = graph __2__ = text
    * `etj_upmoveMeterMaxDelay` - maximum value for each statistics in ms (default __360__)
    * `etj_upmoveMeterGraphX/Y/W/H` - X/Y/W/H of graph display (default __8 8 6 80__)
    * `etj_upmoveMeterGraphColor` - RGBA color of the graph background (default __mdgrey__)
    * `etj_upmoveMeterGraphOnGroundColor` - RGBA color of ground frames on graph (default __green__)
    * `etj_upmoveMeterGraphPreJumpColor` - RGBA color of pre jump frames on graph (default __blue__)
    * `etj_upmoveMeterGraphPostJumpColor` - RGBA color of post jump frames on graph (default __red__)
    * `etj_upmoveMeterGraphOutlineW` - width of graph border (default __1__)
    * `etj_upmoveMeterGraphOutlineColor` - RGBA color of graph border (default __white__)
    * `etj_upmoveMeterTextX` - X offset of text display from graph (default __6__)
    * `etj_upmoveMeterTextH` - vertical spacing between the text displays (default __12__)
    * `etj_upmoveMeterTextColor` - RGBA color of the text display (default __white__)
    * `etj_upmoveMeterTextShadow` - draw shadow on text (default __1__)
    * `etj_upmoveMeterTextSize` - size of text display (default __2__)
* fixed prediction errors with crashlandings which caused delay on applying knockback event & view duck [#812](https://github.com/etjump/etjump/pull/812)
* `!restart` and `reset_match` commands now start the map instantly instead of briefly having a warmup before actual map start [#815](https://github.com/etjump/etjump/pull/815)
* added cvar unlockers for ETe clip/trigger/slick drawing [#813](https://github.com/etjump/etjump/pull/813)
  * requires ETe build from Sep 10th 2022 or newer
  * alternative shaders included in the mod to replace built-in ones from engine
* text shadowing amount is no longer hardcoded to pure pixel values and scales with the text size [#816](https://github.com/etjump/etjump/pull/816)
* added `callvote` devmap and removed several restrictions when running with cheats [#814](https://github.com/etjump/etjump/pull/814)
  * save isn't restricted
  * noclip is always available
  * god is always available
  * cheats are enabled during timeruns (no records are saved)
  * save is available during nosave timeruns, load doesn't reset the run
  * prone isn't restricted
* `!listmaps` and `!findmap` improvements [#825](https://github.com/etjump/etjump/pull/825)
  * `!listmaps` now defaults to 5 columns instead of 3, capped max columns to 5 to prevent output print breaking due to console char limits
  * both `!listmaps` and `!findmap` outputs are now alphabetically ordered and every other column colored grey to improve readability
  * `!findmap` now displays the number of matches similar to `callvote map <partialmapname>`
  * added `!newmaps [N]` command to display `N` latest maps added to server (flag `a`)
* fixed pluralization of some admin command outputs [#826](https://github.com/etjump/etjump/pull/826/files)
* fixed non-functional `savelimit` worldspawn key [#827](https://github.com/etjump/etjump/pull/827)

# ETJump 2.4.0

* fixed connection interrupted icon not moving with lagometer
* added ETPro's extended `cg_drawweaponIconFlash` __2__
  * white - ready
  * yellow - reload/switch
  * red - shooting
* spectator messages (`cg_descriptiveText`) are now hidden when in-game menu is up to improve readability
* unlocked `cg_centertime`
* vote system improvements and fixes
  * fixed calling `callvote` without arguments not printing callvote help
  * `callvote randommap` now correctly respects `g_blockedMaps`
  * added `g_enableVote` to toggle voting
  * removed unused `vote_allow_` cvars
    * added `vote_allow_randommap` to control random map voting
  * voting can now be correctly disabled by setting all `vote_allow_` cvars to 0
  * added `g_spectatorVote` to control spectator voting
    * __1__ spectators can cast votes
    * __2__ spectators can also call votes
    * only spectators who cast votes are count towards `vote_percent` to prevent votes never passing on servers with multiple afk spectators
  * removed in-game vote -> misc menu and replaced it with map restart & random map buttons
  * various string formatting and spelling fixes
  * `callvote map` now lists matched maps if more than one match was found
* added `await` command. Works similar to `wait` except doesn't interrupt other user actions
  * syntax: `await <frames> <command1> | <command2> | <command3>...`
  * `await` is not available during timeruns, and command queue is cleared on timerun starts
* added new color parsing support for crosshair
* fixed a bug in color parser which interpreted hex colors with `00` as single channeled color
* fixed an old etmain bug where changing crosshair via console would not instantly update crosshair preview in menu
* added playercounts to altscoreboard 3 headers
* fixed a bug where players were unable to go to/get up from prone when server reached > 1.07b ms uptime
* fixed a bug that caused demos recorded in ETJump 2.3.0 RC3 and earlier to crash
* added CHS 53 to display Z angle of a plane
* added `etj_extraTrace` cvar to toggle tracing of playerclips on various detectors
  * takes bitmask value
    * __1__ = OB detector
    * __2__ = slick detector
    * __4__ = no jump delay detector
    * __8__ = CHS 10-11
    * __16__ = CHS 12
    * __32__ = CHS 13-15
    * __64__ = CHS 16
    * __128__ = CHS 53
  * the list above can be checked in game with `extraTrace` console command
* character strings now always draw at least 1px high/wide characters even if scaled to very low size
* added spawnflag __1__ `SILENT` to `target/trigger_savereset` to omit the print it makes
* OB watcher no longer shows OBs if you are below your saved Z coordinate and have negative Z velocity
* fixed `func_static` not opening doors when spawnflag __2__ was used
* players who die during timeruns are now put into limbo instantly without leaving a corpse or gun to the floor
* fixed a crash when player reached over 83884088ups while CGaz 2 was enabled
* fixed `setoffset` always adding 1 unit to Z origin
* added shader `deformVertexes` text directive support
* added autodemo support to start recording whenever player loads (unless timerun is active) or spawns
  * `etj_autoDemo 0/1/2` - Enables autodemo for time run maps only (`1`) or for any type of map (`2`).
  * `etj_ad_savePBOnly 0/1` - Save only PBs (`1`) or all demos (`0`) when timerun finishes. Default `0`.
  * `etj_ad_stopDelay <ms>` - Delay demo stop after timerun finishes. Default `2000`. Valid range `0-10000`.
  * `etj_ad_targetPath <path>` - Folder inside demos folder where autodemo saves demos. Default `autodemo` (saves in demos/autodemo).
  * `ad_save <demoname>` - Save currently active demo manually. Format: `playername_map_demoname[DD-MM-YYYY-HHMMSS]`.
  * `/man ad_save` prints usage description.
* fixed unlocked cvars resetting on map changes/restarts
* fixed rounding errors with virtual screen on widescreen resolutions
* extended `etj_popupGrouped`
  * __1__ = group popups, print duplicates in console
  * __2__ = group popups and console prints
* reorganized settings menu
* `ranks <runname>` command now shows time differences relative to your time
* added `etj_speedColorUsesAccel` to color `etj_drawSpeed2` based on accel/decel
* fixed Connection Interrupted message font & scaling to match updated HQ font
* added adrenaline syringe for medics
* fixed a bug where firing a syringe would give you syringe ammo
* updated nosave, noprone, & timerun icons to better match original HUD icons
* fixed chat background drawing a solid line on the right side
* added `etj_chatScale` cvar to control chat size
* fixed color coding breaking on long console prints (such as `/ranks runname`) by making splits at previous newline when message length is > 998 chars
* renamed confusingly named "ghostPlayers" cvars
  * `etj_ghostPlayersAlt` -> `etj_drawSimplePlayers`
  * `etj_ghostPlayersColor` -> `etj_simplePlayersColor`
  * `etj_ghostPlayerOpacity` -> `etj_playerOpacity`
  * `etj_ghostPlayerFadeRange` -> `etj_hideFadeRange`
* tracker debugger fixes
  * added feedback prints when trying to use `tracker_set` or `tracker_print` while `g_debugTrackers` is __0__
  * fixed not being able to set negative tracker values
  * fixed not being able to set tracker value outside of 1-50 when omitting index (setting value to index 1)
* implemented proper prone noclipping
* added `/clearsaves` command to remove your own saves
* fixed limbo menu/fireteam menu shifting to right randomly
* fixed incorrect shaders displaying during loading screen
* `etj_drawCGaz` __2__ no longer shows the side "wings" when movement speed is < 352ups
* fixed `func_invisible_user` playing sounds at world origin (0 0 0)
* fixed incorrect math with `target/trigger_push` spawnflag __2__
  * added spawnflag __4__ `ADD_Z` to `target/trigger_push`
* added `etj_CHSUseFeet` to toggle shifting Z origin to feet level on positional calculations
* added velocity snapping HUD
  * `etj_drawSnapHUD` - Toggle velocity snapping HUD
  * `etj_snapHUDOffsetY` - Y offset for snaphud position
  * `etj_snapHUDHeight` - height of the snaphud
  * `etj_snapHUDFov` - FOV of the snaphud, valid range is __1-180__, __0__ = use current FOV
  * `etj_snapHUDColor1` - 1st RGBA color of snapzones
  * `etj_snapHUDColor2` - 2nd RGBA color of snapzones
* added `etj_drawCGaz` __5__ - equivalent of DeFRaG's `df_hud_cgaz` __13__
  * `etj_CGaz5Color1-4` to set colors of no accel zone, min angle, accel zone and max angle, respectively
  * `etj_CGaz5Fov` to set FOV, valid range is __1-180__, __0__ = use current FOV
* fixed `!rename` passing other client commands to renamed player
* adjusted rendering order of crosshair & crosshairnames
  * crosshair no longer draws when scoreboard is up
* silenced warnings about missing portalgun models
* fixed client's missiles and mines are now removed on timerun start
* changed `etj_altScoreboard` to default to standard scoreboard
* added center print on timerun start if `pmove_fixed` is not enabled
* added `etj_gunSway` to toggle gun idle/movement sway and landing bobbing
* removed unused `etj_drawCGazUsers` cvar
* fixed duplicate popup indicator getting colored based on the popup
* fixed skyportal drawing incorrectly on widescreen aspect when `etj_realFov` was used

# ETJump 2.3.0

* replaced hud/ui low resolution bitmap charset with hq font
* fixed `etj_noActivateLean` not being persistent on respawn causing incorrect behaviour
* added `etj_CGazColor1/2` cvars to set custom colors for `etj_drawCGAZ` __2__
* added `freecam` and `noclip` support for demo playback (compatible with Camtrace3D)
* added `b_demo_lookat` support for demo playback, to viewlock on specific player/entity
* fixed mover tagging bug
* added `g_moverScale` cvar and `!moverScale` command to control mover speed (eg. tank)  
  *note: use flag `v` to control command access*
* added additional scripting player specific commands, expects activator to be player:  
  * `setplayerspawn "objective description"`
  * `setplayerautospawn "objective description"`
  * `damageplayer N`, inflicts __N__ hp damage to the activator
  * `killplayer`
* added new keys for `func_invisible_user` entity:
  * `noise <sound file>` sets sound file to play when activated
  * `volume <0-255>` controls sound volume
* fixed `func_static` spawnflag __2__ (PAIN), direct activation of entities don't crash the game anymore 
* fixed `target_speaker` spawnflag **8** (ACTIVATOR) didn't play sound to the activator
* added `etj_OBX/Y` to move OB detector
* added worldspawn key `nojumpdelay` to control no jump delay behavior on the map
  *note: use `surfaceparm SURF_MONSLICK_N` to enable/disable jump delay*
* added no jump delay surface detector + control cvars:
  * `etj_drawNoJumpDelay`
  * `etj_noJumpDelayX`
  * `etj_noJumpDelayY`
* added save-volumes indicator + control cvars: 
  * `etj_drawSaveIndicator`:
    * __0__ never show icon
    * __1__ always show icon
    * __2__ only show when outside of a save zone
    * __3__ only show when inside of a save zone
  * `etj_saveIndicatorX`
  * `etj_saveIndicatorY`
* fixed footsteps not playing at low landing speeds
  * to revert back to old, broken behavior, toggle `etj_uphillSteps`
* fixed `etj_weaponsound` __0__ not muting no ammo or flamethrower sound
* added `incrementVar` command, works almost like `cycle` but also support floats
* added `strictsaveload` worldspawn key to control save/load behavior
  * bitmask value
  * __1__/__move__ = cannot save while moving
  * __2__/__dead__ = cannot save/load while dead
  * when using strings, seperate values with | (eg. "move | dead")
* added command `setoffset x y z`: offsets player's position by given vector if noclip is available
  * maximum offset value is __4096__
* reworked OB Watcher:
  * OB watcher tracks your movement in air and tells if you are going to get OB on the surface you have saved with `ob_save`
  * `etj_obWatcherSize` to set the size
  * `etj_obWatcherColor` to set the color
* fixed `nooverbounce` allowing stickies on non-OB surfaces
* added ability to spawn and pickup 100% accurate rifles with all classes
* added cvars to force values on locked cvars:
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
* fixed remapped shaders were remaining from previous maps
* added trigger_push support for client predicted push events
* added `etj_touchPickupWeapons` to pickup weapons by touching them:
  * __0__ off
  * __1__ pickup weapons dropped by yourself and spawned in map
  * __2__ pickup any touched weapons
  * functionality relies on having `cg_autoactivate 1`
* fixed `movie_changeFovBasedOnSpeed` toggle not working
* added `noise` key for target_teleporter and trigger_teleport, plays only to client
* added `outspeed` key for target_teleporter and trigger_teleport
  * sets fixed speed at which player exits teleport
  * value __0__ does NOT reset speed, instead ignores the key (default)
* added `etj_consoleAlpha` to control console background transparency
* added `etj_drawLeaves` to toggle original's leaves drawing
* added spawnflag __16__ `KNOCKBACK` to target_teleporter and trigger_teleport
  * applies knockback event for a short time after getting teleported
  * makes teleportation events smoother when destination is on ground level
* fixed `vote_minVoteDuration` not working and votes always taking at least 5 seconds to pass
* added stand models for 100% accurate rifles, spawnflag __2__ now works on them
* added `nofalldamage` worldspawn key to control fall damage behavior
  * `nofalldamage` __0__ = fall damage enabled on all surfaces except on `surfaceparm nodamage` (default)
  * `nofalldamage` __1__ = fall damage disabled on all surfaces except on `surfaceparm nodamage`
  * `nofalldamage` __2__ = fall damage disabled on all surfaces
* added minimize button for game window (Windows only)
* added `etj_keysShadow` to draw shadow for keysets
* added ability to hide popups with `etj_numPopups` __0__
* added spawnflag __1__ `CONSTANT` and key `time` to target_scale_velocity
  * scales activators speed permanently by amount of `scale` for duration of `time`
* added `spawnflag` __128__ `NO_SAVE` for `target_startTimer`
  * disables save during timerun, and when player loads, the run will be reset
* added `target_give` support
* disabled clients ability to rotate z-view
* added z-rotation check when starting a timerun
* fireteam and private messages are no longer logged
* added `etj_autoLoad`. When player joins back to team, the last saved position is automatically loaded.
* demo sorting in replays menu is now case insensitive
* adjusted `etj_slickX` default value so it doesn't overlap with OB detector
* added spawnflag __2048__ to `trigger_multiple`, activates for every client touching the trigger, with per-client wait times
* noclipping is now always allowed on non-dedicated servers running `devmap`
* fixed noclipping as spectator
* added support for triggerable shaders
  * any entity that supports `target` key can have `targetShaderName` and `targetShaderNewName` keys
  * when entity fires its target(s), it will remap `targetShaderName` to `targetShaderNewName`
  * more info: http://robotrenegade.com/q3map2/docs/shader_manual/triggerable-shader-entities.html
* added ability to spectate players by aiming at them and pressing `+activate`. Can be toggled with `etj_quickFollow`
  * value __2__ will also show a hint about spectating when aiming at players
* added `noprone` worldspawn key to enable/disable proning
  * `noprone` __0__: players can only prone outside of `surfaceparm donotenter` (default)
  * `noprone` __1__: players can only prone inside of `surfaceparm donotenter`
* added noprone indicator + control cvars:
  * `etj_drawProneIndicator`:
    * __0__ never show icon
    * __1__ always show icon
    * __2__ only show when outside of a prone zone
    * __3__ only show when inside of a prone zone
  * `etj_proneIndicatorX`
  * `etj_proneIndicatorY`
* added `interruptRun` client command to stop an ongoing timerun
* players now load onto same stance (stand/crouch/prone) as they saved in. Loading to prone additionally forces same viewangles as when saving.
* added `etj_runtimerInactiveColor` to set run timer color when timerun is not active
* added `tracker_not_eq` key to target/trigger_tracker
* fixed runtimer is not displayed in demo playback
* fixed capitalization in noclip prints + colored it yellow
* added `etj_chatLineWidth` to control chat width
  * value indicates characters before line break
  * valid range is 1-200
* increased chat max height to 64 lines
* fixed an issue with difference calculations in timerun prints
* reformatted timerun timer format to `MM:SS.xxx`
* added a new icon for timerun popup prints
* added `etj_loopedSounds` to toggle playback of looped sounds in maps
* improvements to `etj_hideMe`:
  * footsteps, gun firing, pickup, reload and impact sounds are now silent
  * command map/compass doesn't draw icon anymore
  * bullet tracers, brass and wallmarks are hidden
  * demos recorded while hidden now show yourself on freecam/3rd person
    * does NOT draw other players on demo who were also hidden
* added spawnflag __2__ `ADD_XY` to `target/trigger_push`
  * adds the XY velocity from the pusher to your current velocity, rather than setting it
  * when this is set, `speed` key affects the pusher even when its targeting `target_position`
* added etjump menu support for fullscreen ui
* added warning message about serverlist exploit to playonline window
* added etjump servers filter in playonline window
* added whitespace trim for the server names
* disabled automatic serverlist load on playonline window open
* added `etj_chatShadow` cvar to enable chat text shadows
* added `etj_chatAlpha` cvar to set chat transparency
* added `etj_drawCHS2 2` to align chs2 text to the right side of screen
* added `etj_CHS2PosX`, `etj_CHS2PosY` cvars to control chs2 position
* added `etj_CHSShadow` cvar to enable text shadow for CHS
* added `etj_CHSAlpha` cvar to set CHS alpha
* added `etj_CHSColor` cvar to change CHS color
* added `etj_HUD_fireteam` cvar to enable/disable fireteam hud
* added `etj_fireteamPosX`, `etj_fireteamPosY` cvar to position fireteam hud
* added `etj_fireteamAlpha` cvar to set fireteam hud alpha
* added `etj_popupShadow` cvar to set text shadows for popup messages
* added `etj_popupAlpha` cvar to set popup message alpha
* added `etj_HUD_popup` cvar to enable/disable popup message hud, value `2` aligns popup to the right side of screen
* added `etj_popupPosX`, `etj_popupPosY` cvars to position popup message hud
* made keyset to be drawn from the center of the defined origin (using `etj_keysX`, `etj_keysY`)
* added 4 new keyset themes
  * __2__ DeFRaG default
  * __3__ ETJump 2
  * __4__ ETJump 3
  * __5__ ETJump 4
* fixed illegal redirect upon serverlist loading
* added directories support in `replays` menu
* added `etj_consoleShader` cvar to enable/disable textured background
* added `etj_consoleColor` cvar to change console's background color if `etj_consoleShader` is disabled
* fixed random mouse moves bug caused by malicious commands
* fixed portalgun bug, where player was able to teleport and start timerun simultaneously
* added `etj_onRunStart`, `etj_onRunEnd` cvars which can hold sequence of commands to be executed when run events occur
* fixed health regeneration was slower for `pmove_fixed` players
* added `etj_altScoreboard 3`
* added `etj_lagometerX`, `etj_lagometerY` cvars to offset lagometer in x/y directions respectively
* added `g_debugTrackers` to toggle tracker debugging
  * `tracker_print <index1|all> <index2> <index3>...` prints specified tracker index/indices. If index is not specified, defaults to index 1.
  * `tracker_set <index|all> <value>` sets tracker to specified value in specified index. If index is not specified, defaults to index 1.
  * commands only work when `g_debugTrackers` is set to __1__
  * timerun records are not saved when tracker debugging is enabled
* added `g_debugTimeruns` to toggle timerun debugging
  * run start checks are disabled (noclip, speed etc.) 
  * cheats are enabled (noclip, god)
  * no weapons are removed
  * portals are not cleared
  * timerun records are not saved when debugging is enabled
* timeruns no longer start if player is noclipping/has godmode enabled
* prints about disallowed timerun start conditions no longer appear if player is already running
* added high ping checks when timerun is active to prevent lagging through triggers
* fixed timerun view not resetting on map restarts
* removed `etj_speedSizeX/Y` and replaced them with `etj_speedSize`
* added almost all etjump client cvars in the settings menu
* fixed bug, where making `g_banner1-5` cvars empty would cause the server crash on map load
* fixed cursor not showing up in menu text fields
* added new color parsing system for ETJump cvars that expect color values
  * all ETJump cvars expecting color values now support multiple color formats:
    * string (white, black, green etc.)
    * normalized RGB(A) (eg. 1.0 0.5 0.75 0.33)
    * true RGB(A) (eg. 255 128 191 62). If any value is > 1, true RGBA is used instead of normalized
    * hex color (eg. #ff80bf, 0xff80bf)
* fixed `time` option on `!ban` not working and corrected wrong time value in manual
* fixed an issue where using `!deletelevel` to remove your current admin level and using admin commands afterwards would crash server
* fixed `!editlevel` not creating a new level if the level you are trying to edit does not exist
* fixed `!edituser` printing redundant output when using `-clear` option

# ETJump 2.2.0

* Changed all .c modules to .cpp modules.
* Increased progression tracker limit from 10 to 50.
* K43 and Garand no longer have any spread.
* Added spawnflag 8 to `target_startTimer` to reset the runtimer if pmove is not fixed
* Added spawnflag 16 to `target_startTimer` to disable use of save slots and backups
* Portal gun changes:
  * Both portal gun fire rates are now equal.
  * Lowered the minimum allowed distance between the portal gun portals.
  * Changed other players' portal color 1 to green to make it more distinct on light surfaces.
* Enhanced vote UX
  * Spectators can now see the y/n count.
  * Selected y/n is highlighted.
  * Revoting is now possible. Players can change their vote 3 times 10 seconds after the vote.
* Removed unused cvars.
* Vsay now plays the same voice for every player.
* Fixed a click event issue in the widescreen UI.
* `shooter_rocket` can now hit players.
* renamed target_tracker `tracker_setIf` key to `tracker_set_if` to match the other keys.
* fixed long map name not flipped correctly on map loading
* added spawnflags __2__ for `weapon_portalgun` entity to make it rotating around its center
* added spawnflags __4__ for all `item_*` and `weapon_*` entities to make them bobbing if they are floating in the air (__1__)
* `vid_restart` no longer breaks timerun timer.
* `misc_landmine` no longer crashes the client.
* CHS 50 displays the last jump coordinates.
* !spec now correctly only tries to match players that are not spectating.
* Added two new spawnflags for `target_startTimer`
  * __32__ disable explosive weapons pickup
  * __64__ disable portal gun pickup
* Drowned players are now correctly put to spec after a period of inactivity.
* Added worldspawn key `nooverbounce`.  Disables overbounce if set to anything but __0__.
  * Surfaceparm monsterslicksouth enables overbounce on maps with no overbounce enabled.
* Added spawnflag __8__ to `target_teleporter`. Works like __4__ (preserves yaw) except keeps the pitch value as well.
* `trigger_teleporter` now supports same spawnflags as `target_teleporter`.
* Added overbounce watcher. You can save a position and the watcher will display whenever you are pointing at an OB location.
  * `etj_drawObWatcher` to toggle the OB watcher.
  * `etj_obWatcherX/Y` to change the location of the watcher.
  * `ob_save <optional name>` to save the current position. Saved position will be displayed.
  * `ob_load <optional name>` to load the saved position to be displayed. 
  * `ob_reset` to remove the displayed position. (Nothing will be displayed)
* HUD/UI clean up (replaced some drawchars on proper text drawing method)
* Value __9__ for `etj_drawSpeed2` now displays only tens in speedometer (ignores hundreds & thousands)
* Added 'etj_drawMaxSpeed' variable and etj_maxSpeedX/Y/Duration to display max speed from previous load session
* Fixed a bug in `etj_noActivateLean`
* Added `vote_minVoteDuration` to limit how fast a vote can pass after the callvote.

# ETJump 2.1.0

* Extended `target_relay`:  
  * spawnflags __8__: only fires targets if the activating player is currently timerunning  
  * spawnflags __16__: only fires targets if the activating player is currently __NOT__ timerunning
* Added `speed_limit` key to `target_starttimer`.  
  * Timer will not be started if player's speed is higher than the value. Default value is __700__.
* Added `etj_drawTokens`.
* Added `etj_enableTimeruns`.
* Fixed issues with timerun timer.
* Added `trigger_tracker` and `target_tracker`.  
  * A replacement for `target_activate` that's easier to use and allows more complex designs.
* Fixed lines ending with a ^ breaking newlines.
* `g_banners` to enable/disable banners altogether.
* Client side autoexec for map specific configs. (autoexec_mapname.cfg)
* Added spawnflags 1 to `target_remove_portals` to disable the text print.
* Added `target_interrupt_timerun` to stop any timerun without setting a record.
* Added `target_set_health`.  
  * Set's activator's health to the value specified by the health key.  
  * Spawnflags __1__ to set once per life.
* Fixed a likely crash on linux client.
* Added cvars to control ghost player's transparency:  
  * `etj_ghostPlayersOpacity` controls ghostplayer's transparency.  
  * `etj_ghostPlayersFadeRange`  controls distance when ghostplayer starts to fade.
* Custom vsays: `/vsay <variation> <id> <custom text>` e.g. `/vsay 4 hi Good evening!`.
* UI enhancements:
  * optional shadows for runtimer and speed
  * runtimer user experience improved 
  * added cvar to auto hide the timer when it isn't used `etj_runTimerAutoHide`
  * popup message grouping (avoid duplicates) `etj_popupGrouped`
  * teamchat height increased up to 14 lines (from 8)
* Added cvar to control explosives shake `etj_explosivesShake`  
  * __0__ disables shaking, __1__ disables shaking from own explosives, __2__ disables shaking from other player's explosives, __3__ default behaviour
* Widescreen support
* `etj_chatFlags` toggles team flags next to chat messages.
* Improved callvote:  
  * Specs can no longer vote.  
  * Votes will stay for full 30 seconds unless the percentage needed out of number of connected clients has exceeded for either yes or no votes.  
  * For example: 51% => 2 players are in team, 2 in spec => 3 votes are needed. If both players in team vote yes it will wait for 30 seconds and pass. If one of the specs join team vote yes, it will pass instantly. If one of the specs join team and vote no, it will wait for the vote to expire and pass.
* Fixed a bug in listbans.
* Fixed weird coordinates in the chat causing undesired mouse movements.
* Bullets and explosives go through ghost players.
* vsay correctly displays timestamp.
* Added cvar to disable lean when holding `+activate` key along with strafe keys `etj_noActivateLean`.
* Chat improvements:
  * Increased chat messages up to 200 chars
  * New multiline chat textbox with character counter
