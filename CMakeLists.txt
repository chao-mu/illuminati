cmake_minimum_required (VERSION 3.13)
project (demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# My stuff
add_executable(demo src/main.cpp src/App.cpp src/Result.cpp src/ShaderProgram.cpp thirdparty/lodepng/lodepng.cpp)
target_include_directories(demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/thirdparty/lodepng")
target_compile_options(demo PRIVATE "-Wextra" "-Werror" "-Wall" "-pedantic-errors")
target_link_libraries(demo stdc++fs)

# OpenGL
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
target_link_libraries(demo OpenGL::OpenGL)
add_dependencies(demo OpenGL::OpenGL)

# OpenGL - GLFW
find_package(glfw3 3.2 REQUIRED)
target_link_libraries(demo glfw ${GLFW_LIBRARIES})

# OpenGL - GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(demo ${GLEW_LIBRARIES})


# TCLAP
find_package(TCLAP REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})
target_link_libraries(demo ${TCLAP_LIBRARIES})
