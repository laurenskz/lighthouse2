project(Platform)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_library(${PROJECT_NAME} ${sources} ${headers})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 11
                                                 POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})

find_package(FreeImage REQUIRED)
find_package(glfw3 REQUIRED)

if(MSVC)
  # Always use the bundled zlib. Attempting to detect a zlib install on the
  # current machine (eg. find_package(ZLIB QUIET) if ((NOT ZLIB_FOUND)...))
  # sometimes results in finding the one from mingw64, which will not compile
  # natively in Visual Studio.
  set(ZLIB_ROOT "${CMAKE_SOURCE_DIR}/lib/zlib")
endif()

find_package(ZLIB REQUIRED)

# External dependencies, PUBLIC because exposed through header:
target_link_libraries(${PROJECT_NAME} PUBLIC glad ${PLATFORM_GLFW} Half::half
                                             FreeImage::freeimage ZLIB::ZLIB)

if(BUILD_SHARED_LIBS)
  install(TARGETS ${PROJECT_NAME})
endif()
