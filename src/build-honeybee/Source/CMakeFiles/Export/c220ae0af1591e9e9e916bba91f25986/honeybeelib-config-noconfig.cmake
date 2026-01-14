#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "HoneybeeLib" for configuration ""
set_property(TARGET HoneybeeLib APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(HoneybeeLib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libHoneybeeLib.a"
  )

list(APPEND _cmake_import_check_targets HoneybeeLib )
list(APPEND _cmake_import_check_files_for_HoneybeeLib "${_IMPORT_PREFIX}/lib/libHoneybeeLib.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
