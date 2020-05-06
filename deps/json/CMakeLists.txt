add_library(libjson STATIC jsoncpp.cpp)
set_target_properties(libjson PROPERTIES OUTPUT_NAME "json")
target_compile_options(libjson 
					   PRIVATE 
							$<$<CXX_COMPILER_ID:MSVC>:/W0> 
							$<$<CXX_COMPILER_ID:GNU,Clang>:-w>)
target_include_directories(libjson SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
