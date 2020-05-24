# HACK: Install above the data directory, so that CMake install works without
# rewriting relative path loading in all apps.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/..")
