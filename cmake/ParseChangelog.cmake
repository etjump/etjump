macro (parse_changelog CHANGELOG_CVARS CHANGELOG_CVAR_STR)
    file(READ "${CMAKE_SOURCE_DIR}/changelog.md" CONTENTS)

    # split the contents into lines
    string(REPLACE "\n" ";" CONTENTS ${CONTENTS})

    set(SECTION_CONTENT "")
    set(SECTION_NAME "")

    foreach(line ${CONTENTS})
        # is this line a header?
        if(line MATCHES "^# ")
            # save the previous section content
            if(SECTION_NAME AND SECTION_CONTENT)
                set(${SECTION_NAME} "${SECTION_CONTENT}")

                set(FILENAME ${SECTION_NAME})
                string(REPLACE " " "_" FILENAME ${FILENAME})
                string(TOLOWER "${FILENAME}" FILENAME)

                file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui/changelog/${FILENAME}.txt" ${SECTION_NAME} "\n" ${SECTION_CONTENT})

                string(APPEND "${CHANGELOG_CVARS}" ${FILENAME} "|")
                string(REPLACE "ETJump " "" SECTION_NAME ${SECTION_NAME})
                string(APPEND "${CHANGELOG_CVAR_STR}" \"${SECTION_NAME}\" ";" \"${FILENAME}\" ";")

                # debug
                # message("Section: ${SECTION_NAME}")
                # message("Content:\n${SECTION_CONTENT}")
            endif()

            # start a new section
            string(REGEX REPLACE "^# +" "" SECTION_NAME ${line})
            set(SECTION_CONTENT "")
        else()
            # remove all markdown links and trailing whitespace
            string(REGEX REPLACE "(\\ *\\[[^]]*\\]\\([^)]*\\))+" "" CLEANED_LINE "${line}")

            # replace double '**', '__' and backticks with a single quote
            string(REPLACE "**" "'" CLEANED_LINE ${CLEANED_LINE})
            string(REPLACE "__" "'" CLEANED_LINE ${CLEANED_LINE})
            string(REPLACE "`" "'" CLEANED_LINE ${CLEANED_LINE})

            # replace carets with an escaped color string
            string(REPLACE "^" "^^*" CLEANED_LINE ${CLEANED_LINE})

            # replace line starts from '*' to '-'
            string(REGEX REPLACE "^([ \t]*)\\*" "\\1-" CLEANED_LINE ${CLEANED_LINE})

            # append the cleaned line to the current section content
            set(SECTION_CONTENT "${SECTION_CONTENT}\n${CLEANED_LINE}")
        endif()
    endforeach()

    # save the last section
    if(SECTION_NAME AND SECTION_CONTENT)
        set(${SECTION_NAME} "${SECTION_CONTENT}")

        set(FILENAME ${SECTION_NAME})
        string(REPLACE " " "_" FILENAME ${FILENAME})
        string(TOLOWER "${FILENAME}" FILENAME)

        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/assets/ui/changelog/${FILENAME}.txt" ${SECTION_NAME} "\n" ${SECTION_CONTENT})

        string(APPEND "${CHANGELOG_CVARS}" ${FILENAME})
        string(REPLACE "ETJump " "" SECTION_NAME ${SECTION_NAME})
        string(APPEND "${CHANGELOG_CVAR_STR}" \"${SECTION_NAME}\" ";" \"${FILENAME}\")

        # debug
        # message("Section: ${SECTION_NAME}")
        # message("Content:\n${SECTION_CONTENT}")
    endif()
endmacro()
