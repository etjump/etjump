file(GLOB GAME_HEADERS "*.h" "*.hpp")
file(GLOB GAME_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_library(qagame MODULE
	${GAME_SOURCES}
	${GAME_HEADERS}
)
target_compile_definitions(qagame PRIVATE GAMEDLL)
target_link_libraries(qagame PRIVATE cxx_compiler_opts libjson libsha1 libsqlite libsqlite_modern_cpp libuuid4 fmt::fmt toml11::toml11)
target_link_libraries(qagame PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>,,pthread>)
set_target_properties(qagame PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${BASE_DIR_PATH}"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BASE_DIR_PATH}"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BASE_DIR_PATH}")
set_target_platform_details(qagame)
