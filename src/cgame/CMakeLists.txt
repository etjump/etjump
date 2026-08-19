file(GLOB CGAME_HEADERS "*.h" "*.hpp")
file(GLOB CGAME_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_library(cgame MODULE
	${CGAME_HEADERS}
	${CGAME_SOURCES}
	"../game/bg_animation.cpp"
	"../game/bg_animgroup.cpp"
	"../game/bg_character.cpp"
	"../game/bg_classes.cpp"
	"../game/bg_misc.cpp"
	"../game/bg_pmove.cpp"
	"../game/bg_sha1.cpp"
	"../game/bg_slidemove.cpp"
	"../game/bg_sscript.cpp"
	"../game/bg_stats.cpp"
	"../game/bg_tracemap.cpp"
	"../game/q_math.cpp"
	"../game/q_shared.cpp"
	"../game/etj_command_parser.cpp"
	"../game/etj_entity_utilities_shared.cpp"
	"../game/etj_file.cpp"
	"../game/etj_filesystem.cpp"
	"../game/etj_json_utilities.cpp"
	"../game/etj_savepos_shared.cpp"
	"../game/etj_string_utilities.cpp"
	"../game/etj_time_utilities.cpp"
	"../game/etj_timerun_shared.cpp"
	"../game/etj_syscall_ext_shared.cpp"
	"../ui/ui_shared.cpp"
)
target_compile_definitions(cgame PRIVATE CGAMEDLL)
target_link_libraries(cgame PRIVATE cxx_compiler_opts libjson libsha1 libuuid4 fmt::fmt toml11::toml11)

set_target_properties(cgame PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${BASE_DIR_PATH}"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BASE_DIR_PATH}"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BASE_DIR_PATH}")
set_target_platform_details(cgame)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
	set_target_properties(cgame PROPERTIES
			VS_DEBUGGER_COMMAND "${ET_PATH}\\${ET_EXE_NAME}"
			VS_DEBUGGER_COMMAND_ARGUMENTS "+set fs_basepath . +set fs_homepath \"${ET_PATH}\" +set fs_game ${CMAKE_PROJECT_NAME}"
			VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)"
	)
endif ()

if(APPLE)
	# These are just for generating the hardware id
	find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
	find_library(COREF_FRAMEWORK CoreFoundation REQUIRED)
	target_link_libraries(cgame PRIVATE ${IOKIT_FRAMEWORK} ${COREF_FRAMEWORK})
endif()
