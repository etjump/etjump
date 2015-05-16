project(etjump)
cmake_minimum_required(VERSION 2.8.1)
cmake_policy(SET CMP0015 NEW)

# Release
SET(CMAKE_BUILD_TYPE Release)

# Store compiled binaries to folder
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Set the used compiler
# SET(CMAKE_C_COMPILER gcc-3.4)
# SET(CMAKE_CXX_COMPILER g++-3.4)

SET(ARCH "i386")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

IF(${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-DNDEBUG)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=i686")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winline")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finline-functions")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fschedule-insns2")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Release")

SET(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cgame)
SET(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/game)
SET(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ui)

FILE(GLOB CGAME_SRC
	"cgame/cg_atmospheric.c"
	"cgame/cg_character.c"
	"cgame/cg_commandmap.c"
	"cgame/cg_consolecmds.c"
	"cgame/cg_debriefing.c"
	"cgame/cg_draw.c"
	"cgame/cg_drawCHS.c"
	"cgame/cg_drawtools.c"
	"cgame/cg_effects.c"
	"cgame/cg_ents.c"
	"cgame/cg_event.c"
	"cgame/cg_fireteamoverlay.c"
	"cgame/cg_fireteams.c"
	"cgame/cg_flamethrower.c"
	"cgame/cg_info.c"
	"cgame/cg_limbopanel.c"
	"cgame/cg_loadpanel.c"
	"cgame/cg_localents.c"
	"cgame/cg_main.c"
	"cgame/cg_marks.c"
	"cgame/cg_missionbriefing.c"
	"cgame/cg_multiview.c"
	"cgame/cg_newDraw.c"
	"cgame/cg_particles.c"
	"cgame/cg_players.c"
	"cgame/cg_playerstate.c"
	"cgame/cg_polybus.c"
	"cgame/cg_popupmessages.c"
	"cgame/cg_predict.c"
	"cgame/cg_scoreboard.c"
	"cgame/cg_servercmds.c"
	"cgame/cg_snapshot.c"
	"cgame/cg_sound.c"
	"cgame/cg_spawn.c"
	"cgame/cg_statsranksmedals.c"
	"cgame/cg_syscalls.c"
	"cgame/cg_trails.c"
	"cgame/cg_view.c"
	"cgame/cg_weapons.c"
	"cgame/cg_window.c"
	"cgame/cg_manual.c"
	"sha-1/bg_sha.c"
	"sha-1/sha1.c"
	"cgame/cg_identification.cpp"
	"game/bg_animation.c"
	"game/bg_animgroup.c"
	"game/bg_character.c"
	"game/bg_classes.c"
	"game/bg_misc.c"
	"game/bg_pmove.c"
	"game/bg_slidemove.c"
	"game/bg_sscript.c"
	"game/bg_stats.c"
	"game/bg_tracemap.c"
	"game/q_math.c"
	"game/q_shared.c"
	"ui/ui_shared.c"
)

FILE(GLOB QAGAME_SRC
	"game/admin/session.cpp"
	"game/admin/game.cpp"
	"game/admin/commands.cpp"
	"game/admin/database.cpp"
	"game/admin/levels.cpp"
	"game/admin/user.cpp"
	"game/mainext.cpp"
	"game/mapdata.cpp"
	"game/races.cpp"
	"game/custommapvotes.cpp"
	"game/asyncoperation.cpp"
	"game/asyncoperations.cpp"
	"game/loadrace.cpp"
	"game/operationqueue.cpp"
	"game/motd.cpp"
	"game/randommapmode.cpp"
	"game/g_utilities.cpp"
	"game/g_banner.cpp"
	"game/g_save.cpp"
	"game/SQLiteWrapper.cpp"
	"game/Timerun.cpp"
	"sha-1/bg_sha.c"
	"sha-1/sha1.c"
	"sqlite/sqlite3.c"
	"json/jsoncpp.cpp"
	"game/bg_animation.c"
	"game/bg_animgroup.c"
	"game/bg_campaign.c"
	"game/bg_character.c"
	"game/bg_classes.c"
	"game/bg_misc.c"
	"game/bg_pmove.c"
	"game/bg_slidemove.c"
	"game/bg_sscript.c"
	"game/bg_stats.c"
	"game/bg_tracemap.c"
	"game/g_active.c"
	"game/g_alarm.c"
	"game/g_antilag.c"
	"game/g_buddy_list.c"
	"game/g_character.c"
	"game/g_client.c"
	"game/g_cmds.c"
	"game/g_cmds_ext.c"
	"game/g_combat.c"
	"game/g_fireteams.c"
	"game/g_items.c"
	"game/g_main.c"
	"game/g_match.c"
	"game/g_mem.c"
	"game/g_misc.c"
	"game/g_missile.c"
	"game/g_mover.c"
	"game/g_props.c"
	"game/g_referee.c"
	"game/g_script.c"
	"game/g_script_actions.c"
	"game/g_session.c"
	"game/g_spawn.c"
	"game/g_stats.c"
	"game/g_sv_entities.c"
	"game/g_svcmds.c"
	"game/g_syscalls.c"
	"game/g_systemmsg.c"
	"game/g_target.c"
	"game/g_team.c"
	"game/g_teammapdata.c"
	"game/g_trigger.c"
	"game/g_utils.c"
	"game/g_vote.c"
	"game/g_weapon.c"
	"game/q_math.c"
	"game/q_shared.c"
	"game/Printer.cpp"
	"game/Printer.h"
	"game/Utilities.cpp"
	"game/Utilities.h"
	"game/Worker.cpp"
	"game/Worker.h"
)

FILE(GLOB UI_SRC
	"ui/ui_atoms.c"
	"ui/ui_gameinfo.c"
	"ui/ui_loadpanel.c"
	"ui/ui_main.c"
	"ui/ui_players.c"
	"ui/ui_shared.c"
	"ui/ui_syscalls.c"
	"ui/ui_util.c"
	"game/bg_campaign.c"
	"game/bg_classes.c"
	"game/bg_misc.c"
	"game/q_math.c"
	"game/q_shared.c"
)

# cgame library building
add_library (cgame.mp.${ARCH} SHARED ${CGAME_SRC} cgame/Timerun.cpp cgame/Timerun.h)
set_target_properties(cgame.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "CGAMEDLL" PREFIX "")

add_library (qagame.mp.${ARCH} SHARED ${QAGAME_SRC} game/Users.cpp game/Users.h game/User.cpp game/User.h)
set_target_properties(qagame.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "GAMEDLL" PREFIX "")
target_link_libraries(qagame.mp.${ARCH} pthread)

# ui library building
add_library (ui.mp.${ARCH} SHARED ${UI_SRC})
set_target_properties(ui.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "UIDLL" PREFIX "")
