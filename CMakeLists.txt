cmake_minimum_required(VERSION 3.10)
project(XylonEngine)

# Set policies
cmake_policy(SET CMP0072 NEW)

# Source, object, and binary directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(OBJ_DIR ${CMAKE_BINARY_DIR}/obj)
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)

# Executable name
set(EXEC XylonEngine)

# Automatically detect all source files (all .cpp files in src and subdirectories)
file(GLOB_RECURSE SOURCES
    ${SRC_DIR}/*.cpp
)

foreach(TO_REMOVE ${SOURCES})
    if(${TO_REMOVE} MATCHES "${SRC_DIR}/lib/")
        list(REMOVE_ITEM SOURCES ${TO_REMOVE})
    endif()
endforeach()

# Find external packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Include directories
include_directories(
    ${SRC_DIR}/core
    ${SRC_DIR}/console
    ${SRC_DIR}/fonts
    ${SRC_DIR}/def
    ${SRC_DIR}/player
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

# Libraries to link
set(LIBS
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    SDL2_image
    SDL2_mixer
    SDL2_ttf
)

# Create executable
add_executable(${EXEC} ${SOURCES})

# Link libraries
target_link_libraries(${EXEC} ${LIBS})

# Set output directories
set_target_properties(${EXEC} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# Visual Studio specific settings (if needed)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXEC})

# Add Debug/Release build settings
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Clean build directories efficiently
add_custom_target(clean_project
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${OBJ_DIR} ${BIN_DIR}
    COMMENT "Cleaning build directories"
)
