# Copyright (c) 2018, Facebook, Inc.
# All rights reserved.
#
# Find libgflags
#
#  LIBGFLAGS_INCLUDE_DIR - where to find gflags/gflags.h, etc.
#  LIBGFLAGS_LIBRARY     - List of libraries when using libgflags.
#  LIBGFLAGS_FOUND       - True if libgflags found.


IF (LIBGFLAGS_INCLUDE_DIR)
  # Already in cache, be silent
  SET(LIBGFLAGS_FIND_QUIETLY TRUE)
ENDIF ()

FIND_PATH(LIBGFLAGS_INCLUDE_DIR gflags/gflags.h)

FIND_LIBRARY(LIBGFLAGS_LIBRARY_DEBUG NAMES gflagsd gflags_staticd)
FIND_LIBRARY(LIBGFLAGS_LIBRARY_RELEASE NAMES gflags gflags_static)

INCLUDE(SelectLibraryConfigurations)
SELECT_LIBRARY_CONFIGURATIONS(LIBGFLAGS)

# handle the QUIETLY and REQUIRED arguments and set LIBGFLAGS_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(gflags DEFAULT_MSG LIBGFLAGS_LIBRARY LIBGFLAGS_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBGFLAGS_LIBRARY LIBGFLAGS_INCLUDE_DIR)

if (NOT TARGET gflags)
  add_library(gflags UNKNOWN IMPORTED)
  set_target_properties(gflags PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LIBGFLAGS_INCLUDE_DIR}")
  set_target_properties(gflags PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C" IMPORTED_LOCATION "${LIBGFLAGS_LIBRARY}")
endif()
