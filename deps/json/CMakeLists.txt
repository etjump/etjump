add_library(libjson OBJECT jsoncpp.cpp)
add_compile_definitions(JSON_IS_AMALGAMATION)
target_include_directories(libjson SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
