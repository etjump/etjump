add_library(libsha1 STATIC sha1.c)
set_target_properties(libsha1 PROPERTIES OUTPUT_NAME "sha1")
target_compile_options(libsha1 PRIVATE $<IF:$<CXX_COMPILER_ID:MSVC>, /W0, -w>)
target_include_directories(libsha1 SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
