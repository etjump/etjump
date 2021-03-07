add_library(libboost INTERFACE)
set(Boost_LIBRARIES libboost)
target_include_directories(libboost SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
