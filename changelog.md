# ETJump 2.1.0

* Extended target_relay
spawnflags 8: only fires targets if the activating player is currently timerunning
spawnflags 16: only fires targets if the activating player is currently NOT timerunning
* Added speed_limit key to target_starttimer. Timer will not be started if player's speed is higher than the value. Default value is 700.
* Added etj_drawTokens.
* Added etj_enableTimeruns.
* Fixed issues with timerun timer.
* Added trigger_ and target_tracker. A replacement for target_activate that's easier to use and allows more complex designs.
* Fixed lines ending with a ^ breaking newlines.
* `g_banners` to enable/disable banners altogether.
* Client side autoexec for map specific configs. (autoexec_mapname.cfg)
* Added spawnflags 1 to target_remove_portals to disable the text print.
* Added `target_interrupt_timerun`to stop any timerun without setting a record.
* Added `target_set_health`
Set's activator's health to the value specified by the health key.
Spawnflags 1 to set once per life.
* Fixed a likely crash on linux client.
* Custom vsays: /vsay <id> <variation> <custom text> e.g. /vsay hi 4 Good evening!
* UI enhancements:
Optional shadows for timers etc.
Run timer improved.
Explosive shake effect can now be turned off entirely, for other peoples explosions or own explosions.
* Widescreen support
* popup message grouping (no duplicates)
* `etj_chatFlags` to toggle team flags next to chat messages.
* Improved callvote
Specs can no longer vote.
Votes will stay for full 30 seconds unless the percentage needed out of number of connected clients has exceeded for either yes or no votes.
For example: 51% => 2 players are in team, 2 in spec => 3 votes are needed. If both players in team vote yes it will wait for 30 seconds and pass. If one of the specs join team vote yes, it will pass instantly. If one of the specs join team and vote no, it will wait for the vote to expire and pass.
* Fixed a bug in listbans.
* Bullets and explosives go through ghost players
* vsay correctly displays timestamp
* `etj_noActivateLean` to disable lean when pressing `+activate` key