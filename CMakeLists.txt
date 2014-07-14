cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")

project(smf_view)
add_executable(smf_view main.cpp Model.h Model.cpp)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLUI REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GLUI_INCLUDE_DIRS} )
target_link_libraries(smf_view ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLUI_LIBRARY} )
