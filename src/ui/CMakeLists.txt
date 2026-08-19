file(GLOB UI_HEADERS "*.h" "*.hpp")
file(GLOB UI_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_library(ui MODULE
	${UI_HEADERS}
	${UI_SOURCES}
	"../cgame/etj_color_parser.cpp"
	"../game/bg_classes.cpp"
	"../game/bg_misc.cpp"
	"../game/bg_sha1.cpp"
	"../game/q_math.cpp"
	"../game/q_shared.cpp"
	"../game/etj_file.cpp"
	"../game/etj_filesystem.cpp"
	"../game/etj_json_utilities.cpp"
	"../game/etj_string_utilities.cpp"
	"../game/etj_syscall_ext_shared.cpp"
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GenerateMenuHashes)

generate_menu_hashes(ui)

target_compile_definitions(ui PRIVATE UIDLL)
target_link_libraries(ui PRIVATE cxx_compiler_opts libjson libsha1 fmt::fmt)
set_target_properties(ui PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${BASE_DIR_PATH}"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BASE_DIR_PATH}"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BASE_DIR_PATH}")
set_target_platform_details(ui)
