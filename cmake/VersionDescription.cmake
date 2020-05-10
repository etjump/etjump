find_package(Git)

macro(get_version_description VAR)
	if(Git_FOUND)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} describe --tags
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
			OUTPUT_VARIABLE GIT_DESCRIPTION
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		set("${VAR}" "${GIT_DESCRIPTION}")
		string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)\\-([0-9]+)\\-(.+)" OUT_VAR "${GIT_DESCRIPTION}")
		set("${VAR}_MAJOR" "${CMAKE_MATCH_1}")
		set("${VAR}_MINOR" "${CMAKE_MATCH_2}")
		set("${VAR}_PATCH" "${CMAKE_MATCH_3}")
		set("${VAR}_COMMIT" "${CMAKE_MATCH_4}")
		set("${VAR}_COMMIT_HASH" "${CMAKE_MATCH_5}")
	else()
		set("${VAR}" "${CMAKE_PROJECT_VERSION}")
		set("${VAR}_MAJOR" "${CMAKE_PROJECT_VERSION_MAJOR}")
		set("${VAR}_MINOR" "${CMAKE_PROJECT_VERSION_MINOR}")
		set("${VAR}_PATCH" "${CMAKE_PROJECT_VERSION_PATCH}")
		set("${VAR}_COMMIT" "0")
		set("${VAR}_COMMIT_HASH" "0")
	endif()
endmacro()
