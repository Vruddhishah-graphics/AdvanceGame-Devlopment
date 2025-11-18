cmake_minimum_required(VERSION 3.20)
project(FinalProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_definitions(-DGLM_FORCE_RADIANS)

file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/include/**/*.h")
add_executable(FinalProject ${SRC_FILES} ${HEADER_FILES} "include/game/Node.h" "include/game/Texture.h")

# >>>> UPDATE these two variables if your local paths differ <<<<
set(LIB_INCLUDE "D:/CANADA- PB - AI & ml/SEM 3/Advanced Game & Devlopement/Libraries/Libraries/include")
set(LIB_FOLDER  "D:/CANADA- PB - AI & ml/SEM 3/Advanced Game & Devlopement/Libraries/Libraries/lib")

target_include_directories(FinalProject PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${LIB_INCLUDE}"
    "${LIB_INCLUDE}/GL"
    "${LIB_INCLUDE}/GLFW"
    "${LIB_INCLUDE}/glm"
    "${LIB_INCLUDE}/SOIL"
)

find_package(OpenGL REQUIRED)
target_link_libraries(FinalProject PRIVATE
    ${OPENGL_gl_LIBRARY}
    glu32.lib
    "${LIB_FOLDER}/glew32s.lib"
    "${LIB_FOLDER}/glfw3.lib"
    "${LIB_FOLDER}/SOIL.lib"
)

target_compile_definitions(FinalProject PRIVATE
    GLEW_STATIC
    ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
)

if (WIN32)
    set(CMAKE_SUPPRESS_REGENERATION TRUE)
    set_target_properties(FinalProject PROPERTIES DEBUG_POSTFIX "_d")
endif()

message(STATUS "✅ FinalProject configured successfully!")
message(STATUS "Include folder: ${LIB_INCLUDE}")
message(STATUS "Lib folder: ${LIB_FOLDER}")
message(STATUS "Assets folder: ${CMAKE_SOURCE_DIR}/assets")
