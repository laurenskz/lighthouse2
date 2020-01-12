project(RenderCore_Minimal)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_library(${PROJECT_NAME} MODULE ${sources} ${headers})
if(MSVC)
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                               "${CMAKE_SOURCE_DIR}/coredlls")
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE COREDLL_EXPORTS=1)
target_link_libraries(${PROJECT_NAME} PRIVATE RenderSystem)

if(BUILD_SHARED_LIBS)
  install(TARGETS ${PROJECT_NAME})
endif()
