set(PK_NAME "${CMAKE_PROJECT_NAME}-${GAME_VERSION}")
set(PK_PATH "${BASE_DIR_PATH}/${PK_NAME}.pk3")

file(GLOB_RECURSE PK_ASSETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
list(REMOVE_ITEM PK_ASSETS CMakeLists.txt)

message(STATUS "Coping assets/ to ${BASE_DIR_PATH}")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${BASE_DIR_PATH}")
file(GENERATE OUTPUT "${BASE_DIR_PATH}/description.txt" CONTENT "${PK_NAME}")
list(APPEND PK_ASSETS "description.txt")

add_custom_target(mod_pk3
	COMMAND ${CMAKE_COMMAND} -E tar "cf" ${PK_PATH} --format=zip -- "cgame*.so" "ui*.so" ${PK_ASSETS}
	WORKING_DIRECTORY ${BASE_DIR_PATH}
	DEPENDS cgame ui
	COMMENT "Packing ${PK_NAME}.pk3"
)

set(RELEASE_NAME "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}")
set(RELEASE_PATH "${CMAKE_BINARY_DIR}/${RELEASE_NAME}.zip")
add_custom_target(mod_release
	COMMAND ${CMAKE_COMMAND} -E tar "cf" ${RELEASE_PATH} --format=zip -- "${BASE_DIR}/qagame*.so" "${BASE_DIR}/${PK_NAME}.pk3"
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	DEPENDS cgame qagame ui mod_pk3
	COMMENT "Packing ${RELEASE_NAME}.zip"
)
