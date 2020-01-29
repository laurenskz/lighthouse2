project(ImguiApp)

file(GLOB_RECURSE sources "*.cpp")
file(GLOB_RECURSE headers "*.h")
add_executable(${PROJECT_NAME} ${sources} ${headers})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17
                                                 CXX_STANDARD_REQUIRED ON)

# External dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

# Internal dependencies:
target_link_libraries(${PROJECT_NAME} PRIVATE RenderSystem)

register_app(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
                                                 "${CMAKE_CURRENT_SOURCE_DIR}")

install(TARGETS ${PROJECT_NAME} RUNTIME)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
                  "${CMAKE_CURRENT_SOURCE_DIR}/shaders" TYPE DATA)
