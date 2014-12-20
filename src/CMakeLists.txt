project(etjump)
cmake_minimum_required(VERSION 2.8.1)
cmake_policy(SET CMP0015 NEW)

# config start ----------------------------------------------
# -----------------------------------------------------------
# set this to Debug or Release
SET(CMAKE_BUILD_TYPE Release)

# output build results in desired directory
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# compiler settings
# edit these out if you want to let cmake decide which compiler to use
#SET(CMAKE_C_COMPILER gcc-3.4)
#SET(CMAKE_CXX_COMPILER g++-3.4)
# -----------------------------------------------------------
# config end ------------------------------------------------

#
# from here edit at own risk!
#

# building 32 bit libraries
SET(CMAKE_C_FLAGS "-m32")
SET(CMAKE_CXX_FLAGS "-m32")

# arch
SET(ARCH "i386")

# Debug configuration
IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	add_definitions(-DDEBUG)
	add_definitions(-D_DEBUG)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Debug")

# Release configuration
IF(${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-DNDEBUG)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=i686")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winline")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finline-functions")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fschedule-insns2")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i686")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winline")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-functions")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fschedule-insns2")
ENDIF(${CMAKE_BUILD_TYPE} MATCHES "Release")

# at all times turn this off
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")




# global scope of all source files
FILE(GLOB CGAME_SRC
    "cgame/cg_identification.cpp"


	"game/bg_public.h"
	"cgame/cg_local.h"
	"cgame/cg_public.h"
	"ui/keycodes.h"
	"game/q_shared.h"
	"game/surfaceflags.h"
	"cgame/tr_types.h"
	"ui/ui_shared.h"
	"cgame/cg_atmospheric.c"
	"cgame/cg_character.c"
	"cgame/cg_commandmap.c"
	"cgame/cg_consolecmds.c"
	"cgame/cg_debriefing.c"
	"cgame/cg_draw.c"
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
	"cgame/cg_manual.c"
	"cgame/cg_marks.c"
	"cgame/cg_missionbriefing.c"
	"cgame/cg_multiview.c"
	"cgame/cg_newDraw.c"
	"cgame/cg_panelhandling.c"
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
	"game/q_math.c"
	"game/q_shared.c"
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
	"ui/ui_shared.c"
)

FILE(GLOB QAGAME_SRC
    "game/g_local.hpp"

    "game/admin/commands.cpp"
    "game/admin/database.cpp"
    "game/admin/databaseoperation.cpp"
    "game/admin/game.cpp"
    "game/admin/levels.cpp"
    "game/admin/session.cpp"

    "game/asyncoperation.cpp"
    "game/asyncoperations.cpp"
    "game/custommapvotes.cpp"
    "game/g_banner.cpp"
    "game/g_save.cpp"
    "game/g_timerun.cpp"
    "game/g_utilities.cpp"
    "game/loadrace.cpp"
    "game/mainext.cpp"
    "game/mapdata.cpp"
    "game/mapsettings.cpp"
    "game/operationqueue.cpp"
    "game/races.cpp"
    "game/session.cpp"
    "game/timeruns.cpp"

    "json/jsoncpp.cpp"

    "sqlite/sqlite3.c"

    "sha-1/sha1.c"
    "sha-1/bg_sha.c"

	"game/bg_local.h"
	"game/bg_public.h"
	"game/g_local.h"
	"game/g_public.h"
	"game/g_team.h"
	"game/q_shared.h"
	"game/surfaceflags.h"
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
	"game/g_config.c"
	"game/g_database.c"
	"game/g_fireteams.c"
	"game/g_items.c"
	"game/g_main.c"
	"game/g_match.c"
	"game/g_mem.c"
	"game/g_misc.c"
	"game/g_missile.c"
	"game/g_mover.c"
	"game/g_multiview.c"
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

	"game/g_powerups.c"
	"game/g_route.c"
	"game/utilityfunctions.cpp"
)

FILE(GLOB UI_SRC
	"game/bg_public.h"
	"ui/keycodes.h"
	"game/q_shared.h"
	"game/surfaceflags.h"
	"cgame/tr_types.h"
	"ui/ui_local.h"
	"ui/ui_public.h"
	"ui/ui_shared.h"
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
add_library (cgame.mp.${ARCH} SHARED ${CGAME_SRC})
set_target_properties(cgame.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "CGAMEDLL" PREFIX "")

# game library building
IF(OMNIBOT_SUPPORT)
	add_definitions(-DOMNIBOT_SUPPORT)
	include_directories(add-ons/omnibot/Common)
	include_directories(add-ons/omnibot/ET)
	SET(QAGAME_SRC ${QAGAME_SRC} ${OMNIBOT_SRC})
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
ENDIF(OMNIBOT_SUPPORT)

IF(LUA_SUPPORT)
	add_definitions(-DLUA_SUPPORT)
	include_directories(add-ons/lua5.1/include)
	link_directories(add-ons/lua5.1/bin)
	SET(QAGAME_SRC ${QAGAME_SRC} game/g_lua.h)
	SET(QAGAME_SRC ${QAGAME_SRC} game/g_lua.c)
	SET(QAGAME_SRC ${QAGAME_SRC} ${SHA1_SRC})
ENDIF(LUA_SUPPORT)

IF(XPSAVE_SUPPORT)
	add_definitions(-DXPSAVE_SUPPORT)
	include_directories(add-ons/sqlite3/include)
	link_directories(add-ons/sqlite3/bin)
	SET(QAGAME_SRC ${QAGAME_SRC} game/g_xpsave.c)
ENDIF(XPSAVE_SUPPORT)

add_library (qagame.mp.${ARCH} SHARED ${QAGAME_SRC})
set_target_properties(qagame.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "GAMEDLL" PREFIX "")

IF(LUA_SUPPORT)
	target_link_libraries(qagame.mp.${ARCH} lua5.1)
ENDIF(LUA_SUPPORT)

IF(XPSAVE_SUPPORT)
	target_link_libraries(qagame.mp.${ARCH} sqlite3)
ENDIF(XPSAVE_SUPPORT)

# ui library building
add_library (ui.mp.${ARCH} SHARED ${UI_SRC})
set_target_properties(ui.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "UIDLL" PREFIX "")

