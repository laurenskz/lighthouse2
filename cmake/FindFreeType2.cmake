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

# TODO: Add included headers and Windows libs
