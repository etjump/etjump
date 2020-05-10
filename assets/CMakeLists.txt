execute_process(
	COMMAND git describe --tags
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	OUTPUT_VARIABLE GIT_TAG
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(PK_NAME "${CMAKE_PROJECT_NAME}-${GIT_TAG}")
set(PK_PATH "${BASE_DIR}/${PK_NAME}.pk3")

file(GLOB_RECURSE PK_ASSETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
list(REMOVE_ITEM PK_ASSETS CMakeLists.txt)

message(STATUS "Coping assets/ to ${BASE_DIR}")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${BASE_DIR}")
file(GENERATE OUTPUT "${BASE_DIR}/description.txt" CONTENT "${PK_NAME}")
list(APPEND PK_ASSETS "description.txt")

add_custom_target(mod_pk3
	COMMAND ${CMAKE_COMMAND} -E tar "cfv" ${PK_PATH} --format=zip -- $<TARGET_FILE_NAME:cgame> $<TARGET_FILE_NAME:ui> ${PK_ASSETS}
	WORKING_DIRECTORY ${BASE_DIR}
	DEPENDS cgame ui
	COMMENT "Packing ${PK_NAME}.pk3"
)
