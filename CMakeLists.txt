cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(etjump VERSION 2.3 LANGUAGES C CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(BUILD_I386 "Builds i386 binaries" ON)

if(BUILD_I386)
	set(ARCH "i386")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=i686")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
else()
	set(ARCH "x86_64")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -Wall -fmessage-length=0 -std=c99 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	add_definitions(-DNDEBUG)
	# This has to be set to compile with -ffast-math
	# Makes sqlite to use std isNan function instead of own implementation
	add_definitions(-DSQLITE_HAVE_ISNAN)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winline")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffast-math")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fomit-frame-pointer")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -finline-functions")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fschedule-insns2")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
endif()

set(CMAKE_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/src/cgame)
set(CMAKE_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/src/game)
set(CMAKE_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/src/ui)

file(GLOB CGAME_SRC
	"src/cgame/cg_atmospheric.cpp"
	"src/cgame/cg_character.cpp"
	"src/cgame/cg_commandmap.cpp"
	"src/cgame/cg_consolecmds.cpp"
	"src/cgame/cg_debriefing.cpp"
	"src/cgame/cg_draw.cpp"
	"src/cgame/cg_drawCHS.cpp"
	"src/cgame/cg_drawtools.cpp"
	"src/cgame/cg_effects.cpp"
	"src/cgame/cg_ents.cpp"
	"src/cgame/cg_edv.cpp"
	"src/cgame/etj_autodemo_recorder.cpp"
	"src/cgame/etj_awaited_command_handler.cpp"
	"src/cgame/etj_client_commands_handler.cpp"
	"src/cgame/etj_console_alpha.cpp"
	"src/cgame/etj_client_authentication.cpp"
	"src/cgame/etj_cvar_master_drawer.cpp"
	"src/cgame/etj_cvar_shadow.cpp"
	"src/cgame/etj_cvar_update_handler.cpp"
	"src/cgame/etj_demo_recorder.cpp"
	"src/cgame/etj_draw_leaves_handler.cpp"
	"src/cgame/etj_event_loop.cpp"
	"src/cgame/etj_inline_command_parser.cpp"
	"src/cgame/etj_keyset_drawer.cpp"
	"src/cgame/etj_keyset_keybind_drawer.cpp"
	"src/cgame/etj_keyset_system.cpp"
	"src/cgame/etj_entity_events_handler.cpp"
	"src/cgame/etj_maxspeed.cpp"
	"src/cgame/etj_operating_system_linux.cpp"
	"src/cgame/etj_overbounce_watcher.cpp"
	"src/cgame/etj_player_events_handler.cpp"
	"src/cgame/etj_quick_follow_drawable.cpp"
	"src/cgame/etj_speed_drawable.cpp"
	"src/cgame/etj_trickjump_lines.cpp"
	"src/cgame/etj_utilities.cpp"
	"src/cgame/cg_event.cpp"
	"src/cgame/cg_fireteamoverlay.cpp"
	"src/cgame/cg_fireteams.cpp"
	"src/cgame/cg_flamethrower.cpp"
	"src/cgame/cg_identification.cpp"
	"src/cgame/cg_info.cpp"
	"src/cgame/cg_limbopanel.cpp"
	"src/cgame/cg_loadpanel.cpp"
	"src/cgame/cg_localents.cpp"
	"src/cgame/cg_main.cpp"
	"src/cgame/cg_mainext.cpp"
	"src/cgame/cg_manual.cpp"
	"src/cgame/cg_marks.cpp"
	"src/cgame/cg_missionbriefing.cpp"
	"src/cgame/cg_multiview.cpp"
	"src/cgame/cg_newDraw.cpp"
	"src/cgame/cg_particles.cpp"
	"src/cgame/cg_players.cpp"
	"src/cgame/cg_playerstate.cpp"
	"src/cgame/cg_polybus.cpp"
	"src/cgame/cg_popupmessages.cpp"
	"src/cgame/cg_predict.cpp"
	"src/cgame/cg_scoreboard.cpp"
	"src/cgame/cg_servercmds.cpp"
	"src/cgame/cg_snapshot.cpp"
	"src/cgame/cg_sound.cpp"
	"src/cgame/cg_spawn.cpp"
	"src/cgame/cg_statsranksmedals.cpp"
	"src/cgame/cg_syscalls.cpp"
	"src/cgame/cg_timerun.cpp"
	"src/cgame/cg_trails.cpp"
	"src/cgame/cg_view.cpp"
	"src/cgame/cg_weapons.cpp"
	"src/cgame/cg_window.cpp"
	"src/cgame/etj_drawable.cpp"
	"src/cgame/etj_timerun_view.cpp"
	"src/game/etj_string_utilities.cpp"
	"src/game/etj_file.cpp"
	"src/game/etj_filesystem.cpp"
	"src/game/etj_time_utilities.cpp"
	"src/game/bg_animation.cpp"
	"src/game/bg_animgroup.cpp"
	"src/game/bg_character.cpp"
	"src/game/bg_classes.cpp"
	"src/game/bg_misc.cpp"
	"src/game/bg_pmove.cpp"
	"src/game/bg_sha.cpp"
	"src/game/bg_slidemove.cpp"
	"src/game/bg_sscript.cpp"
	"src/game/bg_stats.cpp"
	"src/game/bg_tracemap.cpp"
	"src/game/q_math.cpp"
	"src/game/q_shared.cpp"
	"src/ui/ui_shared.cpp"
)

file(GLOB QAGAME_SRC
	"src/game/bg_animation.cpp"
	"src/game/bg_animgroup.cpp"
	"src/game/bg_campaign.cpp"
	"src/game/bg_character.cpp"
	"src/game/bg_classes.cpp"
	"src/game/bg_misc.cpp"
	"src/game/bg_sha.cpp"
	"src/game/bg_pmove.cpp"
	"src/game/bg_slidemove.cpp"
	"src/game/bg_sscript.cpp"
	"src/game/bg_stats.cpp"
	"src/game/bg_tracemap.cpp"
	"src/game/etj_async_operation.cpp"
	"src/game/etj_banner_system.cpp"
	"src/game/etj_command_variables.cpp"
	"src/game/etj_command_parser.cpp"
	"src/game/etj_command_system.cpp"
	"src/game/etj_commands.cpp"
	"src/game/etj_common.h"
	"src/game/etj_custom_map_votes.cpp"
	"src/game/etj_database.cpp"
	"src/game/etj_deathrun_system.cpp"
	"src/game/etj_entity_utilities.cpp"
	"src/game/etj_file.cpp"
	"src/game/etj_filesystem.cpp"
	"src/game/etj_result_set_formatter.cpp"
	"src/game/etj_game.cpp"
	"src/game/etj_levels.cpp"
	"src/game/etj_main.cpp"
	"src/game/etj_main_ext.cpp"
	"src/game/etj_map_statistics.cpp"
	"src/game/etj_motd.cpp"
	"src/game/etj_operation_queue.cpp"
	"src/game/etj_printer.cpp"
	"src/game/etj_progression_tracker.cpp"
	"src/game/etj_progression_tracker_parser.cpp"
	"src/game/etj_random_map_mode.cpp"
	"src/game/etj_save_system.cpp"
	"src/game/etj_session.cpp"
	"src/game/etj_sqlite_wrapper.cpp"
	"src/game/etj_timerun.cpp"
	"src/game/etj_time_utilities.cpp"
	"src/game/etj_tokens.cpp"
	"src/game/etj_user.cpp"
	"src/game/etj_utilities.cpp"
	"src/game/etj_string_utilities.cpp"
	"src/game/g_active.cpp"
	"src/game/g_alarm.cpp"
	"src/game/g_antilag.cpp"
	"src/game/g_buddy_list.cpp"
	"src/game/g_character.cpp"
	"src/game/g_client.cpp"
	"src/game/g_cmds.cpp"
	"src/game/g_cmds_ext.cpp"
	"src/game/g_combat.cpp"
	"src/game/g_fireteams.cpp"
	"src/game/g_items.cpp"
	"src/game/g_main.cpp"
	"src/game/g_match.cpp"
	"src/game/g_mem.cpp"
	"src/game/g_misc.cpp"
	"src/game/g_missile.cpp"
	"src/game/g_mover.cpp"
	"src/game/g_props.cpp"
	"src/game/g_script.cpp"
	"src/game/g_script_actions.cpp"
	"src/game/g_session.cpp"
	"src/game/g_spawn.cpp"
	"src/game/g_stats.cpp"
	"src/game/g_sv_entities.cpp"
	"src/game/g_svcmds.cpp"
	"src/game/g_syscalls.cpp"
	"src/game/g_systemmsg.cpp"
	"src/game/g_target.cpp"
	"src/game/g_team.cpp"
	"src/game/g_teammapdata.cpp"
	"src/game/g_trigger.cpp"
	"src/game/g_utilities.cpp"
	"src/game/g_utils.cpp"
	"src/game/g_vote.cpp"
	"src/game/g_weapon.cpp"
	"src/game/q_math.cpp"
	"src/game/q_shared.cpp"
)

file(GLOB UI_SRC
	"src/cgame/etj_utilities.cpp"
	"src/game/bg_campaign.cpp"
	"src/game/bg_classes.cpp"
	"src/game/bg_misc.cpp"
	"src/game/q_math.cpp"
	"src/game/q_shared.cpp"
	"src/ui/ui_atoms.cpp"
	"src/ui/ui_gameinfo.cpp"
	"src/ui/ui_loadpanel.cpp"
	"src/ui/ui_main.cpp"
	"src/ui/ui_players.cpp"
	"src/ui/ui_shared.cpp"
	"src/ui/ui_syscalls.cpp"
	"src/ui/ui_util.cpp"
)

find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED)
find_package(GTest REQUIRED)

add_subdirectory(deps/json)
add_subdirectory(deps/sha-1)

add_library(cgame SHARED ${CGAME_SRC})
target_compile_definitions(cgame PRIVATE CGAMEDLL)
set_target_properties(cgame PROPERTIES OUTPUT_NAME cgame.mp.${ARCH} PREFIX "")
target_link_libraries(cgame PRIVATE libjson libsha1 Boost::boost)

add_library(game SHARED ${QAGAME_SRC})
target_compile_definitions(game PRIVATE GAMEDLL)
set_target_properties(game PROPERTIES OUTPUT_NAME qagame.mp.${ARCH} PREFIX "")
target_link_libraries(game PRIVATE pthread libjson libsha1 SQLite::SQLite3 Boost::boost)

add_library(ui SHARED ${UI_SRC})
target_compile_definitions(ui PRIVATE UIDLL)
set_target_properties(ui PROPERTIES OUTPUT_NAME ui.mp.${ARCH} PREFIX "")
target_link_libraries(ui PRIVATE Boost::boost)

enable_testing()

add_executable(tests_all 
			   src/cgame/etj_client_commands_handler.cpp
			   src/cgame/etj_entity_events_handler.cpp
			   src/cgame/etj_utilities.cpp
			   src/cgame/etj_inline_command_parser.cpp
			   src/game/etj_command_parser.cpp
			   src/game/etj_deathrun_system.cpp
			   src/game/etj_deathrun_system.cpp
			   src/game/etj_string_utilities.cpp
			   src/game/q_math.cpp
			   tests/client_commands_handler_tests.cpp
			   tests/color_string_parser_tests.cpp
			   tests/command_parser_tests.cpp
			   tests/deathrun_system_tests.cpp
			   tests/entity_events_handler_tests.cpp
			   tests/inline_command_parser_tests.cpp
			   tests/string_utilities_tests.cpp)
target_link_libraries(tests_all GTest::gtest_main libsha1)
target_compile_options(tests_all 
					   PRIVATE 
							$<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang>>:-ggdb>)
gtest_discover_tests(tests_all)
