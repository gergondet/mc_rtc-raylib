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

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX .html)

  find_description_package(mc_env_description)
  find_description_package(mc_int_obj_description)
  find_description_package(jvrc_description)

  set(ENV_ALIASES_OUT ${PROJECT_BINARY_DIR}/assets/robots/aliases/env.yaml)
  set(OBJECT_ALIASES_OUT ${PROJECT_BINARY_DIR}/assets/robots/aliases/object.yaml)
  file(WRITE ${ENV_ALIASES_OUT} "")
  file(WRITE ${OBJECT_ALIASES_OUT} "")
  file(GLOB URDF_FILES ${MC_ENV_DESCRIPTION_PATH}/urdf/*.urdf)
  foreach(URDF ${URDF_FILES})
    get_filename_component(URDF_NAME "${URDF}" NAME_WE)
    file(APPEND ${ENV_ALIASES_OUT} "env/${URDF_NAME}: [env, /assets/mc_env_description, ${URDF_NAME}]\n")
  endforeach()
  file(GLOB URDF_FILES ${MC_INT_OBJ_DESCRIPTION_PATH}/urdf/*.urdf)
  foreach(URDF ${URDF_FILES})
    get_filename_component(URDF_NAME "${URDF}" NAME_WE)
    file(APPEND ${ENV_ALIASES_OUT} "env/${URDF_NAME}: [env, /assets/mc_int_obj_description, ${URDF_NAME}]\n")
    file(APPEND ${OBJECT_ALIASES_OUT} "object/${URDF_NAME}: [object, /assets/mc_int_obj_description, ${URDF_NAME}]\n")
  endforeach()

  file(COPY etc/mc_rtc.yaml DESTINATION ${PROJECT_BINARY_DIR}/assets/etc/)
  file(COPY ${MC_RTC_LIBDIR}/mc_controller/etc DESTINATION ${PROJECT_BINARY_DIR}/assets/controllers/)
  file(COPY ${JVRC_DESCRIPTION_PATH} DESTINATION ${PROJECT_BINARY_DIR}/assets/)
  file(COPY ${MC_ENV_DESCRIPTION_PATH} DESTINATION ${PROJECT_BINARY_DIR}/assets/)
  file(COPY ${MC_INT_OBJ_DESCRIPTION_PATH} DESTINATION ${PROJECT_BINARY_DIR}/assets/)
  file(COPY ${MC_RTC_BINDIR}/../share/doc/mc_rtc/json/schemas DESTINATION ${PROJECT_BINARY_DIR}/assets/)

  # FSM controller configuration
  file(COPY ${MC_RTC_LIBDIR}/mc_controller/fsm/states/data DESTINATION ${PROJECT_BINARY_DIR}/assets/controllers/fsm/states/)
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/FSM.conf CTL_CONF)
  string(REPLACE "${MC_RTC_LIBDIR}/mc_controller" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/FSM.conf "${CTL_CONF}")

  # DoorSample configuration
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/DoorSample.yaml CTL_CONF)
  string(REPLACE "${MC_RTC_LIBDIR}/mc_controller" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/DoorSample.yaml "${CTL_CONF}")

  # LIPMStabilizer configuration
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/LIPMStabilizer.yaml CTL_CONF)
  string(REPLACE "${MC_RTC_LIBDIR}/mc_controller" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/LIPMStabilizer.yaml "${CTL_CONF}")

  # AdmittanceSample configuration
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/AdmittanceSample.yaml CTL_CONF)
  string(REPLACE "${MC_RTC_LIBDIR}/mc_controller" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/AdmittanceSample.yaml "${CTL_CONF}")

  # LIPMWalking configuration
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/LIPMWalking.yaml CTL_CONF)
  string(REPLACE "${MC_RTC_LIBDIR}/mc_controller" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/LIPMWalking.yaml "${CTL_CONF}")

  # Find panda package
  find_package(mc_panda)
  if(mc_panda_FOUND)
    set(MC_PANDA_SHDIR "${MC_PANDA_INSTALL_PREFIX}/share/mc_panda")
    file(COPY ${MC_PANDA_SHDIR} DESTINATION ${PROJECT_BINARY_DIR}/assets/)
    if(NOT EXISTS ${PROJECT_BINARY_DIR}/assets/mc_panda/meshes/visual)
      find_description_package(franka_description)
      file(COPY ${FRANKA_DESCRIPTION_PATH}/meshes/visual DESTINATION ${PROJECT_BINARY_DIR}/assets/franka_description/meshes/)
    endif()
    file(GLOB URDF_FILES ${PROJECT_BINARY_DIR}/assets/mc_panda/*.urdf)
    foreach(URDF ${URDF_FILES})
      file(READ ${URDF} URDF_CONTENT)
      string(REPLACE "${MC_PANDA_SHDIR}" "/assets/mc_panda" URDF_CONTENT "${URDF_CONTENT}")
      file(WRITE ${URDF} "${URDF_CONTENT}")
    endforeach()
  endif()
endif()

find_package(assimp REQUIRED)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games
set(SUPPORT_FILEFORMAT_BMP ON CACHE BOOL "" FORCE) # Support for loading bmp files
set(SUPPORT_FILEFORMAT_JPG ON CACHE BOOL "" FORCE) # Support for loading jpg files
if(EMSCRIPTEN)
  set(PLATFORM "Web" CACHE STRING "" FORCE)
endif()
add_subdirectory(raylib)
if(EMSCRIPTEN)
  target_compile_options(raylib_static PUBLIC "-matomics" -s USE_PTHREADS=1)
endif()

set(imgui_SRC
  imgui/imgui.h
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_opengl3.h
  imgui/backends/imgui_impl_opengl3.cpp
  imgui_impl_raylib.cpp
  imgui_impl_raylib.h
)

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
  SceneState.cpp
  widgets/Category.h
  widgets/Category.cpp
  widgets/Widget.h
  widgets/Label.h
  widgets/Robot.h
  widgets/Robot.cpp
  widgets/Point3D.h
  widgets/Point3D.cpp
  widgets/form/widgets.cpp
  widgets/Schema.cpp
  widgets/form/schema.cpp
  ${imgui_SRC}
)

file(COPY raylib/src/external/glad.h DESTINATION "${PROJECT_BINARY_DIR}/include/glad/")

add_executable(main ${main_SRC})
target_compile_definitions(main PUBLIC R3D_ASSIMP_SUPPORT)
target_link_libraries(main PUBLIC raylib ${ASSIMP_LIBRARIES})
if(EMSCRIPTEN)
  set_target_properties(main PROPERTIES OUTPUT_NAME index)
  set_target_properties(main PROPERTIES COMPILE_FLAGS "-matomics -s USE_PTHREADS=1 -s DISABLE_EXCEPTION_CATCHING=0")
  target_link_libraries(main PUBLIC "-s USE_GLFW=3")
  target_link_libraries(main PUBLIC IrrXML "--shell-file ${PROJECT_SOURCE_DIR}/index.html" "--preload-file assets" "-s USE_PTHREADS=1" "-s PTHREAD_POOL_SIZE=8" "-Wl,--error-limit=0" "-s DISABLE_EXCEPTION_CATCHING=0" "-s TOTAL_MEMORY=512MB" "-matomics" "-s USE_ZLIB=1" "-s ALLOW_MEMORY_GROWTH=1" "--source-map-base http://localhost:8000/" "-Wl,--whole-archive")
else()
  target_compile_definitions(main PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
endif()
target_link_libraries(main PUBLIC mc_rtc::mc_control_fsm mc_rtc::mc_control_client)
if(TARGET mc_rtc::mc_rtc_ros)
  target_compile_definitions(main PUBLIC MC_RTC_HAS_ROS_SUPPORT)
  target_link_libraries(main PUBLIC mc_rtc::mc_rtc_ros)
endif()
target_include_directories(main PUBLIC
  ${ASSIMP_INCLUDE_DIRS}
  "${PROJECT_SOURCE_DIR}/raylib-3D"
  "${PROJECT_SOURCE_DIR}/imgui"
  "${PROJECT_SOURCE_DIR}/imgui/backends"
  "${PROJECT_BINARY_DIR}/include/"
)
