macro(generate_menu_hashes target)
  file(GLOB MENU_FILES "${CMAKE_SOURCE_DIR}/assets/ui/*.menu")
  set(HASH_STRING_LIST "")

  foreach(file IN LISTS MENU_FILES)
    file(SHA1 "${file}" file_hash)
    cmake_path(GET file FILENAME file_name)

    if(HASH_STRING_LIST STREQUAL "")
      set(HASH_STRING_LIST "${file_name}:${file_hash}")
    else()
      set(HASH_STRING_LIST "${HASH_STRING_LIST},${file_name}:${file_hash}")
    endif()
  endforeach()

  target_compile_definitions(${target} PRIVATE MENU_HASH_LIST="${HASH_STRING_LIST}")
endmacro()
