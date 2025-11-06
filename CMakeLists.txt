cmake_minimum_required(VERSION 3.20)

# Project name
project(CannonDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SRCS
    main.cpp
)

# Executable
add_executable(CannonDemo ${SRCS})

# Find OpenGL
find_package(OpenGL REQUIRED)

# Path to your libraries
set(LIB_FOLDER "D:/CANADA- PB - AI & ml/SEM 3/Advanced Game & Devlopement/Libraries/Libraries/lib")
set(INCLUDE_FOLDER "D:/CANADA- PB - AI & ml/SEM 3/Advanced Game & Devlopement/Libraries/Libraries/include")

# Include directories
target_include_directories(CannonDemo PRIVATE
    ${INCLUDE_FOLDER}
    ${INCLUDE_FOLDER}/GL
    ${INCLUDE_FOLDER}/GLFW
    ${INCLUDE_FOLDER}/glm
    ${INCLUDE_FOLDER}/SOIL
)

# Link libraries with full paths
target_link_libraries(CannonDemo PRIVATE
    ${OPENGL_gl_LIBRARY}    # links opengl32.lib
    glu32.lib               # <-- add this to fix unresolved GLU functions
    "${LIB_FOLDER}/glew32s.lib"
    "${LIB_FOLDER}/glfw3.lib"
    "${LIB_FOLDER}/SOIL.lib"
)

# Define GLEW static (needed if you are linking glew32s.lib)
target_compile_definitions(CannonDemo PRIVATE GLEW_STATIC)

# Windows specific tweaks
if(WIN32)
    set(CMAKE_SUPPRESS_REGENERATION TRUE)
    set_target_properties(CannonDemo PROPERTIES DEBUG_POSTFIX _d)
endif()
