#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "dpp::dpp" for configuration ""
set_property(TARGET dpp::dpp APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(dpp::dpp PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libdpp.so.10.0.27"
  IMPORTED_SONAME_NOCONFIG "libdpp.so.10.0.27"
  )

list(APPEND _IMPORT_CHECK_TARGETS dpp::dpp )
list(APPEND _IMPORT_CHECK_FILES_FOR_dpp::dpp "${_IMPORT_PREFIX}/lib/libdpp.so.10.0.27" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
