cmake_minimum_required(VERSION 3.20)
project(FinalProject)
add_definitions(-DASSET_DIR="${CMAKE_SOURCE_DIR}/assets")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----- SAFEST FIX -----
# Build always happens inside C:/_safe_build (no "&" in path)
set(SAFE_BUILD_DIR "C:/_safe_build/FinalProject")
file(MAKE_DIRECTORY ${SAFE_BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SAFE_BUILD_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SAFE_BUILD_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SAFE_BUILD_DIR})
# -----------------------

# Libraries path (your original)
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
    include/game/PBRMaterial.h 
    include/game/SkeletalAnimation.h 
    include/game/ShadowMap.h 
    include/game/PostProcess.h
)

# Copy assets to SAFE build folder
add_custom_command(
    TARGET FinalProject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${SAFE_BUILD_DIR}/assets"
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
