cmake_minimum_required(VERSION 3.21) 
project (lxqt_windowlist_wayland_toplevel VERSION 0.1)

#set(CMAKE_CXX_STANDARD 20)

include(FindPkgConfig)

pkg_check_modules(SYSTEMD REQUIRED libsystemd>=0.34)

list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/protocols")

#message(STATUS "Build protocols")
#execute_process(COMMAND bash build-protocols.sh
#  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/protocols
#  RESULT_VARIABLE protocols_result
#  OUTPUT_VARIABLE protocols_output)
#message(STATUS "Protocols [${protocols_result}]: ${protocols_output}")

add_executable(lxqt_windowlist_wayland_toplevel 
  src/dbus.c  
  src/main.c       
  src/wayland_window_actions.c  
  src/window.c                  
  src/windowlist.c
#  src_protocols/foreign-toplevel.c
)

include_directories(${RSVG_INCLUDE_DIRS} ${EXTRA_INCLUDES} "${PROJECT_BINARY_DIR}")
target_link_libraries(lxqt_windowlist_wayland_toplevel wayland-client ${SYSTEMD_LIBRARIES})

install(TARGETS lxqt_windowlist_wayland_toplevel DESTINATION bin)
