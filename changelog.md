# ETJump 2.3.0

* replaced hud/ui low resolution bitmap charset with hq font
* fixed `etj_noActivateLean` not being persistent on respawn causing incorrect behaviour
* added `etj_CGazColor1/2` cvars to set custom colors for `etj_drawCGAZ` __2__
* added `freecam` and `noclip` support for demo playback (compatible with Camtrace3D)
* added `b_demo_lookat` support for demo playback, to viewlock on specific player/entity
* fixed mover tagging bug
* added `g_moverScale` cvar and `!moverScale` command to control mover speed (eg. tank)  
  *note: use flag `v` to control command access*
* added new keys for `func_invisible_user` entity:
  - `noise <sound file>` sets sound file to play when activated
  - `volume <0-255>` controls sound volume
* fixed `func_static` spawnflag __2__ (PAIN), direct activation of entities don't crash the game anymore 
* fixed `target_speaker` spawnflag **8** (ACTIVATOR) didn't play sound to the activator
* fixed setting `com_maxfps` in menu
* added `etj_viewPlayerPortals` __2__ as an option to menu
* added `etj_drawKeys` __2__ to show DeFRaG style keys
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
* fixed `etj_weaponsound` __0__ doesn't mute no ammo sound
* added `incrementVar` command, works almost like `cycle` but also support floats
* added `strictsaveload` worldspawn key to control save/load behavior
  * bitmask value
  * __1__/__move__ = cannot save while moving
  * __2__/__dead__ = cannot save/load while dead
  * when using strings, seperate values with | (eg. "move | dead")
* added command `setoffset x y z`: offsets player's position by given vector if noclip is available
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
* fixed `etj_weaponSound` not affecting flamethrower
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
* added 3 new keyset themes, you can switch these using `etj_drawKeys` cvar
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
* added `[F] to follow` hint under the player name, can be toggled off along with the quick follow feature using `etj_quickfollow` cvar
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
* fixed `time` option on `!ban` not working
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
  - Both portal gun fire rates are now equal.
  - Lowered the minimum allowed distance between the portal gun portals.
  - Changed other players' portal color 1 to green to make it more distinct on light surfaces.
* Enhanced vote UX
  - Spectators can now see the y/n count.
  - Selected y/n is highlighted.
  - Revoting is now possible. Players can change their vote 3 times 10 seconds after the vote.
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
  - __32__ disable explosive weapons pickup
  - __64__ disable portal gun pickup
* Drowned players are now correctly put to spec after a period of inactivity.
* Added worldspawn key `nooverbounce`.  Disables overbounce if set to anything but __0__.
  - Surfaceparm monsterslicksouth enables overbounce on maps with no overbounce enabled.
* Added spawnflag __8__ to `target_teleporter`. Works like __4__ (preserves yaw) except keeps the pitch value as well.
* `trigger_teleporter` now supports same spawnflags as `target_teleporter`.
* Added overbounce watcher. You can save a position and the watcher will display whenever you are pointing at an OB location.
  - `etj_drawObWatcher` to toggle the OB watcher.
  - `etj_obWatcherX/Y` to change the location of the watcher.
  - `ob_save <optional name>` to save the current position. Saved position will be displayed.
  - `ob_load <optional name>` to load the saved position to be displayed. 
  - `ob_reset` to remove the displayed position. (Nothing will be displayed)
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
Timer will not be started if player's speed is higher than the value. Default value is __700__.
* Added `etj_drawTokens`.
* Added `etj_enableTimeruns`.
* Fixed issues with timerun timer.
* Added `trigger_tracker` and `target_tracker`.  
A replacement for `target_activate` that's easier to use and allows more complex designs.
* Fixed lines ending with a ^ breaking newlines.
* `g_banners` to enable/disable banners altogether.
* Client side autoexec for map specific configs. (autoexec_mapname.cfg)
* Added spawnflags 1 to `target_remove_portals` to disable the text print.
* Added `target_interrupt_timerun` to stop any timerun without setting a record.
* Added `target_set_health`.  
Set's activator's health to the value specified by the health key.  
Spawnflags __1__ to set once per life.
* Fixed a likely crash on linux client.
* Added cvars to control ghost player's transparency:  
`etj_ghostPlayersOpacity` controls ghostplayer's transparency.  
`etj_ghostPlayersFadeRange`  controls distance when ghostplayer starts to fade.
* Custom vsays: /vsay &lt;variation&gt;  &lt;id&gt;  &lt;custom text&gt; e.g. `/vsay 4 hi Good evening!`.
* UI enhancements:
  * optional shadows for runtimer and speed
  * runtimer user experience improved 
  * added cvar to auto hide the timer when it isn't used `etj_runTimerAutoHide`
  * popup message grouping (avoid duplicates) `etj_popupGrouped`
  * teamchat height increased up to 14 lines (from 8)
* Added cvar to control explosives shake `etj_explosivesShake`  
  __0__ disables shaking, __1__ disables shaking from own explosives, __2__ disables shaking from other player's explosives, __3__ default behaviour
* Widescreen support
* `etj_chatFlags` toggles team flags next to chat messages.
* Improved callvote:  
Specs can no longer vote.  
Votes will stay for full 30 seconds unless the percentage needed out of number of connected clients has exceeded for either yes or no votes.  
For example: 51% => 2 players are in team, 2 in spec => 3 votes are needed. If both players in team vote yes it will wait for 30 seconds and pass. If one of the specs join team vote yes, it will pass instantly. If one of the specs join team and vote no, it will wait for the vote to expire and pass.
* Fixed a bug in listbans.
* Fixed weird coordinates in the chat causing undesired mouse movements.
* Bullets and explosives go through ghost players.
* vsay correctly displays timestamp.
* Added cvar to disable lean when holding `+activate` key along with strafe keys `etj_noActivateLean`.
* Chat improvements:
  * Increased chat messages up to 200 chars
  * New multiline chat textbox with character counter
