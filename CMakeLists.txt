cmake_minimum_required (VERSION 3.13)
project (illum)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# My stuff
add_executable(${PROJECT_NAME} src/main.cpp src/App.cpp src/MathUtil.cpp src/JoystickManager.cpp src/Joystick.cpp src/Result.cpp src/ShaderProgram.cpp src/Webcam.cpp src/Image.cpp src/Size.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_SOURCE_DIR}/thirdparty/lodepng")
target_compile_options(${PROJECT_NAME} PRIVATE "-Wextra" "-Werror" "-Wall" "-pedantic-errors" "-Wconversion")

# Filesystem
include(CheckCXXSymbolExists)
CHECK_CXX_SYMBOL_EXISTS(std::filesystem::path::preferred_separator filesystem cxx17fs)

if (LINUX)
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()

if (APPLE)
    set(LLVM_LIB /usr/local/opt/llvm/lib)
    IF(EXISTS ${LLVM_LIB})
        target_link_libraries(${PROJECT_NAME} "-L${LLVM_LIB}")
    endif()
        
    target_link_libraries(${PROJECT_NAME} -lc++fs)
endif()

# LodePNG
add_subdirectory("${CMAKE_SOURCE_DIR}/thirdparty/lodepng")
target_link_libraries(${PROJECT_NAME} lodepng)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${OPENGL_INCLUDE_DIRS})

# OpenGL - GLFW
find_package(glfw3 3.2 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES}) 
# OpenGL - GLEW

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})

# TCLAP
find_package(TCLAP REQUIRED)
include_directories(${TCLAP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${TCLAP_LIBRARIES})

# yaml-cpp
find_package(yaml-cpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${YAML_CPP_LIBRARIES})

# OpenCV
find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
