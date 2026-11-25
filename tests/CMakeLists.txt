include(GoogleTest)

add_executable(tests
	"../src/cgame/etj_client_commands_handler.cpp"
	"../src/cgame/etj_color_parser.cpp"
	"../src/cgame/etj_entity_events_handler.cpp"
	"../src/cgame/etj_inline_command_parser.cpp"
	"../src/game/etj_command_parser.cpp"
	"../src/game/etj_database_v2.cpp"
	"../src/game/etj_deathrun_system.cpp"
	"../src/game/etj_deathrun_system.cpp"
	"../src/game/etj_fatal_error_shared.cpp"
	"../src/game/etj_game_failure.cpp"
	"../src/game/etj_log.cpp"
	"../src/game/etj_portalgun_shared.cpp"
	"../src/game/etj_savepos_shared.cpp"
	"../src/game/etj_string_utilities.cpp"
	"../src/game/etj_timerun_shared.cpp"
	"../src/game/etj_user_repository.cpp"
	"../src/game/q_math.cpp"
	"client_commands_handler_tests.cpp"
	"color_string_parser_tests.cpp"
	"command_parser_tests.cpp"
	"crypto_tests.cpp"
	"deathrun_system_tests.cpp"
	"entity_events_handler_tests.cpp"
	"enum_bitset_tests.cpp"
	"fatal_error_tests.cpp"
	"game_failure_tests.cpp"
	"inline_command_parser_tests.cpp"
	"keep_alive_tests.cpp"
	"portalgun_shared_tests.cpp"
	"string_utilities_tests.cpp"
	"time_utilities_tests.cpp"
	"timerun_shared_tests.cpp"
	"savepos_shared_tests.cpp"
)
# only exists with NEW_AUTH; gtest_add_tests scans sources without the preprocessor,
# so listing it unconditionally registers tests that run nothing
if (NEW_AUTH)
	target_sources(tests PRIVATE "user_repository_tests.cpp")
endif ()

target_link_libraries(tests PRIVATE gtest_main libsha1 picosha2 libsqlite libsqlite_modern_cpp fmt::fmt cxx_compiler_opts)
target_compile_options(tests PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang>>:-ggdb>)
gtest_add_tests(TARGET tests)
