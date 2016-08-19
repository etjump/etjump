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
* Added two new spawnflags for target_startTimer
  32 disable explosive weapons pickup
  64 disable portal gun pickup

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
