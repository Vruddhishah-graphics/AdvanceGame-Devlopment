cmake_minimum_required(VERSION 3.20)
project(FinalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# AGGRESSIVE x86 compiler settings
if(MSVC)
    # Maximum heap space and disable optimizations that use memory
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm500")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    # Disable debug info to save memory
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi-")
    # Use minimal rebuild
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gm-")
endif()

add_definitions(-DASSET_DIR="${CMAKE_SOURCE_DIR}/assets")

set(LIBRARY_DIR "D:/CANADA- PB - AI & ml/SEM 3/Advanced Game & Devlopement/Libraries/Libraries")

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${LIBRARY_DIR}/include
)

link_directories(${LIBRARY_DIR}/lib)

# Source files
add_executable(FinalProject 
    src/main.cpp
    src/Game.cpp
    src/Camera.cpp
    src/MeshUtils.cpp
    src/ParticleSystem.cpp
    src/LightningSystem.cpp
    src/UIRenderer.cpp
)

if(MSVC)
    add_definitions(-DGLEW_STATIC -DGLM_FORCE_RADIANS -D_CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(FinalProject 
    opengl32
    glew32s
    glfw3
    winmm
)