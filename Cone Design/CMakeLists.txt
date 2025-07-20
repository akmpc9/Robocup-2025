cmake_minimum_required(VERSION 3.20)
project(with_gui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# raylib via homebrew (manual paths bc no cmake config)
execute_process(COMMAND brew --prefix raylib OUTPUT_VARIABLE RAYLIB_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
set(RAYLIB_INCLUDE_DIR "${RAYLIB_PREFIX}/include")
set(RAYLIB_LIBRARY_DIR "${RAYLIB_PREFIX}/lib")
# glm via homebrew
execute_process(COMMAND brew --prefix glm OUTPUT_VARIABLE GLM_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
set(GLM_INCLUDE_DIR "${GLM_PREFIX}/include")
add_executable(game with_gui.cpp)

target_include_directories(game PRIVATE ${RAYLIB_INCLUDE_DIR} ${GLM_INCLUDE_DIR})
target_link_directories(game PRIVATE ${RAYLIB_LIBRARY_DIR})
# macos frameworks
find_library(COREVIDEO_FRAMEWORK CoreVideo)
find_library(IOKIT_FRAMEWORK IOKit)
find_library(COCOA_FRAMEWORK Cocoa)
find_library(OPENGL_FRAMEWORK OpenGL)
find_library(GLUT_FRAMEWORK GLUT)
target_link_libraries(game raylib ${COREVIDEO_FRAMEWORK} ${IOKIT_FRAMEWORK} ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK} ${GLUT_FRAMEWORK})
