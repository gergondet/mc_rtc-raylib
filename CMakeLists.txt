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

find_package(Boost REQUIRED COMPONENTS program_options)

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX .html)

  file(COPY img DESTINATION ${PROJECT_BINARY_DIR})

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
  file(COPY ${MC_CONTROLLER_RUNTIME_INSTALL_PREFIX}/etc DESTINATION ${PROJECT_BINARY_DIR}/assets/controllers/)
  if(EXISTS ${MC_PLUGINS_RUNTIME_INSTALL_PREFIX}/etc)
    file(COPY ${MC_PLUGINS_RUNTIME_INSTALL_PREFIX}/etc DESTINATION ${PROJECT_BINARY_DIR}/assets/plugins/)
  endif()
  if(EXISTS ${MC_PLUGINS_RUNTIME_INSTALL_PREFIX}/autoload)
    file(COPY ${MC_PLUGINS_RUNTIME_INSTALL_PREFIX}/autoload DESTINATION ${PROJECT_BINARY_DIR}/assets/plugins/)
  endif()
  file(COPY ${JVRC_DESCRIPTION_PATH} DESTINATION ${PROJECT_BINARY_DIR}/assets/)
  file(COPY ${MC_ENV_DESCRIPTION_PATH} DESTINATION ${PROJECT_BINARY_DIR}/assets/)
  file(COPY ${MC_INT_OBJ_DESCRIPTION_PATH} DESTINATION ${PROJECT_BINARY_DIR}/assets/)
  file(COPY ${MC_RTC_BINDIR}/../share/doc/mc_rtc/json/schemas DESTINATION ${PROJECT_BINARY_DIR}/assets/)

  # FSM controller configuration
  file(COPY ${MC_CONTROLLER_RUNTIME_INSTALL_PREFIX}/fsm/states/data DESTINATION ${PROJECT_BINARY_DIR}/assets/controllers/fsm/states/)
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/FSM.conf CTL_CONF)
  string(REPLACE "${MC_CONTROLLER_RUNTIME_INSTALL_PREFIX}" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/FSM.conf "${CTL_CONF}")

  # DoorSample configuration
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/DoorSample.yaml CTL_CONF)
  string(REPLACE "${MC_CONTROLLER_RUNTIME_INSTALL_PREFIX}" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/DoorSample.yaml "${CTL_CONF}")

  # LIPMStabilizer configuration
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/LIPMStabilizer.yaml CTL_CONF)
  string(REPLACE "${MC_CONTROLLER_RUNTIME_INSTALL_PREFIX}" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/LIPMStabilizer.yaml "${CTL_CONF}")

  # AdmittanceSample configuration
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/AdmittanceSample.yaml CTL_CONF)
  string(REPLACE "${MC_CONTROLLER_RUNTIME_INSTALL_PREFIX}" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/AdmittanceSample.yaml "${CTL_CONF}")

  # LIPMWalking configuration
  file(READ ${PROJECT_BINARY_DIR}/assets/controllers/etc/LIPMWalking.yaml CTL_CONF)
  string(REPLACE "${MC_CONTROLLER_RUNTIME_INSTALL_PREFIX}" "/assets/controllers" CTL_CONF "${CTL_CONF}")
  file(WRITE ${PROJECT_BINARY_DIR}/assets/controllers/etc/LIPMWalking.yaml "${CTL_CONF}")

  # EndEffector configuration
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/etc/EndEffector.yaml" DESTINATION "${PROJECT_BINARY_DIR}/assets/controllers/etc/")

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

  # Remove unused data from the demo
  set(UNUSED_ENV beam box_table chair corniche crapahut crapahut_bricks drc_terrain platform table table_printer)
  foreach(env ${UNUSED_ENV})
    set(BASE "${PROJECT_BINARY_DIR}/assets/mc_env_description")
    file(REMOVE_RECURSE "${BASE}/convex/${env}")
    file(REMOVE_RECURSE "${BASE}/meshes/${env}")
    file(REMOVE_RECURSE "${BASE}/rsdf/${env}")
    file(REMOVE_RECURSE "${BASE}/urdf/${env}.urdf")
  endforeach()
  set(UNUSED_OBJ box litter pole printer seesaw valve valve_handle)
  foreach(obj ${UNUSED_OBJ})
    set(BASE "${PROJECT_BINARY_DIR}/assets/mc_int_obj_description")
    file(REMOVE_RECURSE "${BASE}/convex/${obj}")
    file(REMOVE_RECURSE "${BASE}/meshes/${obj}")
    file(REMOVE_RECURSE "${BASE}/rsdf/${obj}")
    file(REMOVE_RECURSE "${BASE}/urdf/${obj}.urdf")
  endforeach()
endif()

find_package(assimp REQUIRED)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games
set(SUPPORT_FILEFORMAT_BMP ON CACHE BOOL "" FORCE) # Support for loading bmp files
set(SUPPORT_FILEFORMAT_JPG ON CACHE BOOL "" FORCE) # Support for loading jpg files
if(EMSCRIPTEN)
  set(PLATFORM "Web" CACHE STRING "" FORCE)
endif()
add_subdirectory(raylib EXCLUDE_FROM_ALL)
if(EMSCRIPTEN)
  target_compile_options(raylib_static PUBLIC "-matomics" -s USE_PTHREADS=1)
endif()

set(imgui_SRC
  imgui/imgui.h
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_opengl3.h
  imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${imgui_SRC})
target_include_directories(imgui PUBLIC "${PROJECT_SOURCE_DIR}/imgui" "${PROJECT_SOURCE_DIR}/imgui/backends" "${PROJECT_BINARY_DIR}/include")
if(NOT EMSCRIPTEN)
  target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
  if(APPLE)
    target_compile_definitions(imgui PRIVATE GLAD_IMPLEMENTATION)
  endif()
endif()

set(main_SRC
  main.cpp
  RobotModel.h
  RobotModel.cpp
  InteractiveMarker.h
  InteractiveMarker.cpp
  utils.h
  utils.cpp
  help.h
  help.cpp
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
  imgui_impl_raylib.cpp
  imgui_impl_raylib.h
)

file(COPY raylib/src/external/glad.h DESTINATION "${PROJECT_BINARY_DIR}/include/glad/")

add_executable(${PROJECT_NAME} ${main_SRC})
target_compile_definitions(${PROJECT_NAME} PUBLIC R3D_ASSIMP_SUPPORT)
target_link_directories(${PROJECT_NAME} PUBLIC ${ASSIMP_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC raylib ${ASSIMP_LIBRARIES} imgui)
if(EMSCRIPTEN)
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME index)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-matomics -s USE_PTHREADS=1 -s DISABLE_EXCEPTION_CATCHING=0")
  target_link_libraries(${PROJECT_NAME} PUBLIC "-s USE_GLFW=3")
  target_link_libraries(${PROJECT_NAME} PUBLIC IrrXML "-s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'" "--shell-file ${PROJECT_SOURCE_DIR}/index.html" "--preload-file assets" "-s USE_PTHREADS=1" "-s PTHREAD_POOL_SIZE=8" "-Wl,--error-limit=0" "-s DISABLE_EXCEPTION_CATCHING=0" "-s TOTAL_MEMORY=512MB" "-matomics" "-s USE_ZLIB=1" "-s ALLOW_MEMORY_GROWTH=1" "--source-map-base http://localhost:8000/" "-Wl,--whole-archive")
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC mc_rtc::mc_control_fsm mc_rtc::mc_control_client Boost::program_options)
if(TARGET mc_rtc::mc_rtc_ros)
  target_compile_definitions(${PROJECT_NAME} PUBLIC MC_RTC_HAS_ROS_SUPPORT)
  target_link_libraries(${PROJECT_NAME} PUBLIC mc_rtc::mc_rtc_ros)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC
  ${ASSIMP_INCLUDE_DIRS}
  "${PROJECT_SOURCE_DIR}/raylib-3D"
)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
