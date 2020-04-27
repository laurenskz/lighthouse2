project(Taskflow::Taskflow LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE IMPORTED GLOBAL)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR})

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.17)
  # Transitive cxx 17 compile feature breaks CUDA in CMake<3.17
  # https://gitlab.kitware.com/cmake/cmake/issues/19123
  target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
endif()
