project(CustomCore_tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(Google_Tests_run RaytracerTest.cpp)
target_include_directories(Google_Tests_run PRIVATE "../RenderCore_Custom/include")

target_link_libraries(Google_Tests_run RenderCore_Custom)

target_link_libraries(Google_Tests_run gtest gtest_main)

target_link_libraries(Google_Tests_run RenderSystem)
