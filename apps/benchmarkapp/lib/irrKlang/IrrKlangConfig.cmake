cmake_minimum_required(VERSION 3.0)

find_path(
  IRRKLANG_INCLUDE_DIR
  NAMES irrKlang.h
  PATHS "${CMAKE_CURRENT_LIST_DIR}/include"
  NO_DEFAULT_PATH)

find_library(
  IRRKLANG_LIBRARY
  NAMES IrrKlang
  PATHS "${CMAKE_CURRENT_LIST_DIR}/lib/x64"
  NO_DEFAULT_PATH)

find_library(
  IRRKLANG_PLUGIN_MP3_LIBRARY
  NAMES ikpMP3
  PATHS "${CMAKE_CURRENT_LIST_DIR}/lib/x64" NO_DEFAULT_PATH)

if(WIN32)
  find_file(
    IRRKLANG_RUNTIME_LIBRARY
    NAMES irrKlang.dll
    # HACK: Find in benchmarkapp directory:
    PATHS "${CMAKE_CURRENT_LIST_DIR}/../../"
    NO_DEFAULT_PATH)

  set(_fphsa_runtime_lib IRRKLANG_RUNTIME_LIBRARY)
endif(WIN32)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IrrKlang DEFAULT_MSG IRRKLANG_LIBRARY
                                  IRRKLANG_INCLUDE_DIR ${_fphsa_runtime_lib})

if(IrrKlang_FOUND)
  if(NOT TARGET IrrKlang::irrklang)
    add_library(IrrKlang::irrklang SHARED IMPORTED)
    target_include_directories(IrrKlang::irrklang
                               INTERFACE ${IRRKLANG_INCLUDE_DIR})

    if(WIN32)
      set_target_properties(
        IrrKlang::irrklang
        PROPERTIES IMPORTED_IMPLIB "${IRRKLANG_LIBRARY}"
                   IMPORTED_LOCATION "${IRRKLANG_RUNTIME_LIBRARY}")
    else(WIN32)
      set_target_properties(
        IrrKlang::irrklang PROPERTIES IMPORTED_LOCATION "${IRRKLANG_LIBRARY}"
                                      IMPORTED_NO_SONAME TRUE)
    endif(WIN32)

    # This is a local library bundled with the project. Include it in the
    # installation:
    install(FILES "$<TARGET_PROPERTY:IrrKlang::irrklang,IMPORTED_LOCATION>"
            TYPE LIB)
  endif()
endif()

mark_as_advanced(IRRKLANG_LIBRARY IRRKLANG_INCLUDE_DIR)
