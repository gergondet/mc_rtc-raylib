cmake_minimum_required(VERSION 3.1)

# Prefers OpenGL vendor implementation when available
# https://cmake.org/cmake/help/latest/policy/CMP0072.html
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
  set(CMAKE_POLICY_DEFAULT_CMP0072 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(mc-rtc-raylib LANGUAGES C CXX VERSION 0.1.0)

find_package(mc_rtc REQUIRED)

find_package(assimp REQUIRED)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games
set(SUPPORT_FILEFORMAT_BMP ON CACHE BOOL "" FORCE) # Support for loading bmp files
set(SUPPORT_FILEFORMAT_JPG ON CACHE BOOL "" FORCE) # Support for loading jpg files
add_subdirectory(raylib)

set(main_SRC
  main.cpp
  RobotModel.h
  RobotModel.cpp
  InteractiveMarker.h
  InteractiveMarker.cpp
  utils.h
  utils.cpp
  r3d.c
  Camera.h
  Camera.cpp
  Client.h
  Client.cpp
  SceneState.h
)

add_executable(main ${main_SRC})
target_compile_definitions(main PUBLIC R3D_ASSIMP_SUPPORT)
target_link_libraries(main PUBLIC raylib mc_rtc::mc_control_client mc_rtc::mc_control ${ASSIMP_LIBRARIES})
target_include_directories(main PUBLIC ${ASSIMP_INCLUDE_DIRS} "${PROJECT_SOURCE_DIR}/raylib-3D")
