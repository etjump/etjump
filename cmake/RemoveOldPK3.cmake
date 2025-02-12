if (NOT PK3_PATH)
    message(FATAL_ERROR "PK3_PATH not specified")
endif ()

file(GLOB OLD_PK3_FILES "${PK3_PATH}/etjump-*.pk3")

message("pk3 path: ${PK3_PATH}")

if (NOT OLD_PK3_FILES)
    message("No old PK3 files found.")
else ()
    foreach (PK3 IN LISTS OLD_PK3_FILES)
        message("Removing file ${PK3}")
        execute_process(COMMAND ${CMAKE_COMMAND} -E rm -f ${PK3})
    endforeach ()
endif ()
