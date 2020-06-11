cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0015 NEW)

project(etjump LANGUAGES C CXX)

# Release
if (NOT CMAKE_BUILD_TYPE)
	message(WARNING "Build type is not set, forcing \"Release\"")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Store compiled binaries to folder
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

option(USE_GCC4 "Uses GCC4" ON)
option(BUILD_I386 "Builds i386 binaries" ON)

# Set the used compiler
if(USE_GCC4)
	set(CMAKE_C_COMPILER gcc-4.9)
	set(CMAKE_CXX_COMPILER g++-4.9)
else()
	set(CMAKE_C_COMPILER gcc)
	set(CMAKE_CXX_COMPILER g++)
endif()

if(BUILD_I386)
	set(ARCH "i386")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -march=i686")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -march=i686")
else()
	set(ARCH "x86_64")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -Wall -fmessage-length=0 -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Wall -lstdc++ -std=c++11")

set(cxx_default_flags_release
	-O3
	-flto
	-fPIC 
	-ffast-math 
	-fomit-frame-pointer 
	-fschedule-insns2 
	-fno-strict-aliasing 
	-fvisibility=hidden
)
string(REPLACE ";" " " cxx_default_flags_release "${cxx_default_flags_release}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${cxx_default_flags_release}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${cxx_default_flags_release}")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -O3 -flto -s")

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-DNDEBUG)
	# This has to be set to compile with -ffast-math
	# Makes sqlite to use std isNan function instead of own implementation
	add_definitions(-DSQLITE_HAVE_ISNAN)
endif()

set(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cgame)
set(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/game)
set(CMAKE_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ui)

file(GLOB CGAME_SRC
	"cgame/cg_atmospheric.cpp"
	"cgame/cg_character.cpp"
	"cgame/cg_commandmap.cpp"
	"cgame/cg_consolecmds.cpp"
	"cgame/cg_debriefing.cpp"
	"cgame/cg_draw.cpp"
	"cgame/cg_drawCHS.cpp"
	"cgame/cg_drawtools.cpp"
	"cgame/cg_effects.cpp"
	"cgame/cg_ents.cpp"
	"cgame/cg_edv.cpp"
	"cgame/etj_autodemo_recorder.cpp"
	"cgame/etj_awaited_command_handler.cpp"
	"cgame/etj_client_commands_handler.cpp"
	"cgame/etj_console_alpha.cpp"
	"cgame/etj_client_authentication.cpp"
	"cgame/etj_cvar_master_drawer.cpp"
	"cgame/etj_cvar_shadow.cpp"
	"cgame/etj_cvar_update_handler.cpp"
	"cgame/etj_demo_recorder.cpp"
	"cgame/etj_draw_leaves_handler.cpp"
	"cgame/etj_event_loop.cpp"
	"cgame/etj_inline_command_parser.cpp"
	"cgame/etj_keyset_drawer.cpp"
	"cgame/etj_keyset_keybind_drawer.cpp"
	"cgame/etj_keyset_system.cpp"
	"cgame/etj_entity_events_handler.cpp"
	"cgame/etj_maxspeed.cpp"
	"cgame/etj_operating_system_linux.cpp"
	"cgame/etj_overbounce_watcher.cpp"
	"cgame/etj_player_events_handler.cpp"
	"cgame/etj_quick_follow_drawable.cpp"
	"cgame/etj_speed_drawable.cpp"
	"cgame/etj_trickjump_lines.cpp"
	"cgame/etj_utilities.cpp"
	"cgame/cg_event.cpp"
	"cgame/cg_fireteamoverlay.cpp"
	"cgame/cg_fireteams.cpp"
	"cgame/cg_flamethrower.cpp"
	"cgame/cg_identification.cpp"
	"cgame/cg_info.cpp"
	"cgame/cg_limbopanel.cpp"
	"cgame/cg_loadpanel.cpp"
	"cgame/cg_localents.cpp"
	"cgame/cg_main.cpp"
	"cgame/cg_mainext.cpp"
	"cgame/cg_manual.cpp"
	"cgame/cg_marks.cpp"
	"cgame/cg_missionbriefing.cpp"
	"cgame/cg_multiview.cpp"
	"cgame/cg_newDraw.cpp"
	"cgame/cg_particles.cpp"
	"cgame/cg_players.cpp"
	"cgame/cg_playerstate.cpp"
	"cgame/cg_polybus.cpp"
	"cgame/cg_popupmessages.cpp"
	"cgame/cg_predict.cpp"
	"cgame/cg_scoreboard.cpp"
	"cgame/cg_servercmds.cpp"
	"cgame/cg_snapshot.cpp"
	"cgame/cg_sound.cpp"
	"cgame/cg_spawn.cpp"
	"cgame/cg_statsranksmedals.cpp"
	"cgame/cg_syscalls.cpp"
	"cgame/cg_timerun.cpp"
	"cgame/cg_trails.cpp"
	"cgame/cg_view.cpp"
	"cgame/cg_weapons.cpp"
	"cgame/cg_window.cpp"
	"cgame/etj_drawable.cpp"
	"cgame/etj_timerun_view.cpp"
	"game/etj_string_utilities.cpp"
	"game/etj_file.cpp"
	"game/etj_filesystem.cpp"
	"game/etj_time_utilities.cpp"
	"game/bg_animation.cpp"
	"game/bg_animgroup.cpp"
	"game/bg_character.cpp"
	"game/bg_classes.cpp"
	"game/bg_misc.cpp"
	"game/bg_pmove.cpp"
	"game/bg_slidemove.cpp"
	"game/bg_sscript.cpp"
	"game/bg_stats.cpp"
	"game/bg_tracemap.cpp"
	"game/q_math.cpp"
	"game/q_shared.cpp"
	"sha-1/bg_sha.cpp"
	"sha-1/sha1.c"
	"ui/ui_shared.cpp"
	"json/jsoncpp.cpp"
)

file(GLOB QAGAME_SRC
	"game/bg_animation.cpp"
	"game/bg_animgroup.cpp"
	"game/bg_campaign.cpp"
	"game/bg_character.cpp"
	"game/bg_classes.cpp"
	"game/bg_misc.cpp"
	"game/bg_pmove.cpp"
	"game/bg_slidemove.cpp"
	"game/bg_sscript.cpp"
	"game/bg_stats.cpp"
	"game/bg_tracemap.cpp"
	"game/etj_async_operation.cpp"
	"game/etj_banner_system.cpp"
	"game/etj_command_variables.cpp"
	"game/etj_command_parser.cpp"
	"game/etj_command_system.cpp"
	"game/etj_commands.cpp"
	"game/etj_common.h"
	"game/etj_custom_map_votes.cpp"
	"game/etj_database.cpp"
	"game/etj_deathrun_system.cpp"
	"game/etj_entity_utilities.cpp"
	"game/etj_file.cpp"
	"game/etj_filesystem.cpp"
	"game/etj_result_set_formatter.cpp"
	"game/etj_game.cpp"
	"game/etj_levels.cpp"
	"game/etj_main.cpp"
	"game/etj_main_ext.cpp"
	"game/etj_map_statistics.cpp"
	"game/etj_motd.cpp"
	"game/etj_operation_queue.cpp"
	"game/etj_printer.cpp"
	"game/etj_progression_tracker.cpp"
	"game/etj_progression_tracker_parser.cpp"
	"game/etj_random_map_mode.cpp"
	"game/etj_save_system.cpp"
	"game/etj_session.cpp"
	"game/etj_sqlite_wrapper.cpp"
	"game/etj_timerun.cpp"
	"game/etj_time_utilities.cpp"
	"game/etj_tokens.cpp"
	"game/etj_user.cpp"
	"game/etj_utilities.cpp"
	"game/etj_string_utilities.cpp"
	"game/g_active.cpp"
	"game/g_alarm.cpp"
	"game/g_antilag.cpp"
	"game/g_buddy_list.cpp"
	"game/g_character.cpp"
	"game/g_client.cpp"
	"game/g_cmds.cpp"
	"game/g_cmds_ext.cpp"
	"game/g_combat.cpp"
	"game/g_fireteams.cpp"
	"game/g_items.cpp"
	"game/g_main.cpp"
	"game/g_match.cpp"
	"game/g_mem.cpp"
	"game/g_misc.cpp"
	"game/g_missile.cpp"
	"game/g_mover.cpp"
	"game/g_props.cpp"
	"game/g_script.cpp"
	"game/g_script_actions.cpp"
	"game/g_session.cpp"
	"game/g_spawn.cpp"
	"game/g_stats.cpp"
	"game/g_sv_entities.cpp"
	"game/g_svcmds.cpp"
	"game/g_syscalls.cpp"
	"game/g_systemmsg.cpp"
	"game/g_target.cpp"
	"game/g_team.cpp"
	"game/g_teammapdata.cpp"
	"game/g_trigger.cpp"
	"game/g_utilities.cpp"
	"game/g_utils.cpp"
	"game/g_vote.cpp"
	"game/g_weapon.cpp"
	"game/q_math.cpp"
	"game/q_shared.cpp"
	"json/jsoncpp.cpp"
	"sqlite/sqlite3.c"
	"sha-1/bg_sha.cpp"
	"sha-1/sha1.c"
)

file(GLOB UI_SRC
	"cgame/etj_utilities.cpp"
	"game/bg_campaign.cpp"
	"game/bg_classes.cpp"
	"game/bg_misc.cpp"
	"game/q_math.cpp"
	"game/q_shared.cpp"
	"ui/ui_atoms.cpp"
	"ui/ui_gameinfo.cpp"
	"ui/ui_loadpanel.cpp"
	"ui/ui_main.cpp"
	"ui/ui_players.cpp"
	"ui/ui_shared.cpp"
	"ui/ui_syscalls.cpp"
	"ui/ui_util.cpp"
)

# cgame library building
add_library (cgame.mp.${ARCH} MODULE ${CGAME_SRC})
set_target_properties(cgame.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "CGAMEDLL" PREFIX "")

add_library (qagame.mp.${ARCH} MODULE ${QAGAME_SRC})
set_target_properties(qagame.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "GAMEDLL" PREFIX "")
target_link_libraries(qagame.mp.${ARCH} pthread)

# ui library building
add_library (ui.mp.${ARCH} MODULE ${UI_SRC})
set_target_properties(ui.mp.${ARCH} PROPERTIES COMPILE_DEFINITIONS "UIDLL" PREFIX "")
