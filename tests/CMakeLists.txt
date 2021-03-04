include(GoogleTest)

add_executable(tests 
	"../src/cgame/etj_client_commands_handler.cpp"
	"../src/cgame/etj_entity_events_handler.cpp"
	"../src/cgame/etj_utilities.cpp"
	"../src/cgame/etj_inline_command_parser.cpp"
	"../src/game/etj_command_parser.cpp"
	"../src/game/etj_deathrun_system.cpp"
	"../src/game/etj_deathrun_system.cpp"
	"../src/game/etj_string_utilities.cpp"
	"../src/game/q_math.cpp"
	"client_commands_handler_tests.cpp"
	"color_string_parser_tests.cpp"
	"command_parser_tests.cpp"
	"deathrun_system_tests.cpp"
	"entity_events_handler_tests.cpp"
	"inline_command_parser_tests.cpp"
	"string_utilities_tests.cpp"
)
target_link_libraries(tests PRIVATE gtest_main libsha1 ${Boost_LIBRARIES} cxx_compiler_opts)
target_compile_options(tests PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang>>:-ggdb>)
gtest_add_tests(TARGET tests)
