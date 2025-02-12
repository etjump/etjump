set(PK3_NAME "${CMAKE_PROJECT_NAME}-${GAME_VERSION}")
set(PK3_PATH "${BASE_DIR_PATH}/${PK3_NAME}.pk3")

file(GLOB_RECURSE PK3_ASSETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
list(REMOVE_ITEM PK3_ASSETS CMakeLists.txt)

message(STATUS "Copying assets/ to ${BASE_DIR_PATH}")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${BASE_DIR_PATH}")
file(GENERATE OUTPUT "${BASE_DIR_PATH}/description.txt" CONTENT "${PK3_NAME}")
list(APPEND PK3_ASSETS "description.txt")

add_custom_target(mod_pk3
		COMMAND ${CMAKE_COMMAND}
			-DINPUT="${PK3_ASSETS};cgame*.so;cgame*.dll;cgame_mac;ui*.so;ui*.dll;ui_mac"
			-DOUTPUT="${PK3_PATH}"
			-P ${CMAKE_SOURCE_DIR}/cmake/AssetsPackager.cmake
		WORKING_DIRECTORY ${BASE_DIR_PATH}
		DEPENDS remove_old_pk3
		COMMENT "Packing ${PK3_NAME}.pk3"
)

set(RELEASE_NAME "${CMAKE_PROJECT_NAME}-${GAME_VERSION}")
set(RELEASE_PATH "${CMAKE_BINARY_DIR}/${RELEASE_NAME}.zip")

add_custom_target(mod_release
		COMMAND ${CMAKE_COMMAND}
			-DINPUT="${BASE_DIR}/qagame*.so;${BASE_DIR}/qagame*.dll;${BASE_DIR}/qagame_mac;${BASE_DIR}/${PK3_NAME}.pk3"
			-DOUTPUT="${RELEASE_PATH}"
			-P ${CMAKE_SOURCE_DIR}/cmake/AssetsPackager.cmake
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		DEPENDS mod_pk3
		COMMENT "Packing ${RELEASE_NAME}.zip"
)

add_custom_target(remove_old_pk3
		COMMAND ${CMAKE_COMMAND}
			-DPK3_PATH="${BASE_DIR_PATH}"
			-P ${CMAKE_SOURCE_DIR}/cmake/RemoveOldPK3.cmake
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		COMMENT "Cleaning old mod pk3 files"
)
