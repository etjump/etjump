add_library(libjson STATIC jsoncpp.cpp)
set_target_properties(libjson PROPERTIES OUTPUT_NAME "json")
target_include_directories(libjson INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
