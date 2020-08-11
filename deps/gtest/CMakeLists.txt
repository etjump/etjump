# force MD in Debug
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

add_subdirectory(googletest)

# force MT in Release
target_compile_options(gtest PUBLIC $<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release>:/MT>>)
target_compile_options(gtest_main PUBLIC $<$<CXX_COMPILER_ID:MSVC>:$<$<CONFIG:Release>:/MT>>)
