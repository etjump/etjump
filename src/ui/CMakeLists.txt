add_library(ui SHARED 
	"ui_atoms.cpp"
	"ui_gameinfo.cpp"
	"ui_loadpanel.cpp"
	"ui_main.cpp"
	"ui_players.cpp"
	"ui_shared.cpp"
	"ui_syscalls.cpp"
	"ui_util.cpp"
	"../game/bg_campaign.cpp"
	"../game/bg_classes.cpp"
	"../game/bg_misc.cpp"
	"../game/q_math.cpp"
	"../game/q_shared.cpp"
	"../game/etj_utilities.cpp"
)
target_compile_definitions(ui PRIVATE UIDLL)
set_target_properties(ui PROPERTIES OUTPUT_NAME ui.mp.$<IF:$<STREQUAL:${TARGET_ARCH},x86>,i386,x86_64> PREFIX "")
target_link_libraries(ui PRIVATE cxx_compiler_opts Boost::boost)
