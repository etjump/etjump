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
  - `noise <sound file>` sets sound file to play when activated
  - `volume <0-255>` controls sound volume
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
