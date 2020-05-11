# Packs assets into zip blob
# Runs assets list through glob command to allow glob expression evaluation

if(NOT OUTPUT)
	message(FATAL_ERROR "OUTPUT is not specified")
endif()
# noop
if(NOT INPUT)
	return()
endif()

string(REPLACE " " ";" ASSETS ${INPUT})
file(GLOB GLOBBED_ASSETS RELATIVE ${CMAKE_SOURCE_DIR} ${ASSETS})
execute_process(COMMAND ${CMAKE_COMMAND} -E tar "cf" ${OUTPUT} --format=zip -- ${GLOBBED_ASSETS})
