file(GLOB UI_HEADERS "*.h" "*.hpp")

add_library(ui MODULE
	"etj_colorpicker.cpp"
	"ui_atoms.cpp"
	"ui_gameinfo.cpp"
	"ui_loadpanel.cpp"
	"ui_main.cpp"
	"ui_shared.cpp"
	"ui_syscalls.cpp"
	"ui_util.cpp"
	"../game/bg_classes.cpp"
	"../game/bg_misc.cpp"
	"../game/q_math.cpp"
	"../game/q_shared.cpp"
	"../cgame/etj_utilities.cpp"
	"../game/etj_string_utilities.cpp"
	${UI_HEADERS}
)
target_compile_definitions(ui PRIVATE UIDLL)
target_link_libraries(ui PRIVATE cxx_compiler_opts libsha1 fmt::fmt)
set_target_properties(ui PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${BASE_DIR_PATH}"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BASE_DIR_PATH}"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BASE_DIR_PATH}")
set_target_platform_details(ui)
