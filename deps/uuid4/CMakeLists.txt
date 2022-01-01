add_library(libuuid4 OBJECT uuid4.c)
target_include_directories(libuuid4 SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
