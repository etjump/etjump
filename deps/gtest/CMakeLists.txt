# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

add_subdirectory(googletest)
