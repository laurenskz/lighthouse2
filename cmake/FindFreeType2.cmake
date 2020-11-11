find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(FT2_Dummy IMPORTED_TARGET freetype2)

  if(${FT2_Dummy_FOUND}) # All modules are found
    # Expose the shared library. This relies on the OS providing this libary as
    # well as all dependencies.
    pkg_check_modules(FreeType2 IMPORTED_TARGET GLOBAL freetype2)
    add_library(FreeType2::freetype2 ALIAS PkgConfig::FreeType2)

    # Done.
    return()
  endif()
endif(PkgConfig_FOUND)

set(FreeType2_ROOT "${CMAKE_SOURCE_DIR}/lib/FreeType2")

find_path(FreeType2_INCLUDE_DIR "ft2build.h" PATHS "${FreeType2_ROOT}/include")

find_library(FreeType2_SHARED_LIBRARY freetype PATHS "${FreeType2_ROOT}/win64")

if(WIN32)
  # Not strictly necessary, "wrong" path anyway.
  find_file(FreeType2_SHARED_DLL freetype.dll
            PATHS "${CMAKE_SOURCE_DIR}/apps/imguiapp")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeType2 DEFAULT_MSG FreeType2_INCLUDE_DIR
                                  FreeType2_SHARED_LIBRARY FreeType2_SHARED_DLL)

add_library(FreeType2::freetype2 SHARED IMPORTED)
target_include_directories(FreeType2::freetype2
                           INTERFACE ${FreeType2_INCLUDE_DIR})
if(WIN32)
  set_target_properties(
    FreeType2::freetype2
    PROPERTIES IMPORTED_IMPLIB "${FreeType2_SHARED_LIBRARY}"
               IMPORTED_LOCATION "${FreeType2_SHARED_DLL}")
else()
  set_target_properties(
    FreeType2::freetype2 PROPERTIES IMPORTED_LOCATION
                                    "${FreeType2_SHARED_LIBRARY}")
endif()

install(FILES "$<TARGET_PROPERTY:FreeType2::freetype2,IMPORTED_LOCATION>"
        TYPE LIB)
