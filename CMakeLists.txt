cmake_minimum_required(VERSION 3.16)
project(XylonEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set policies
cmake_policy(SET CMP0072 NEW)

# Directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(OBJ_DIR ${CMAKE_BINARY_DIR}/obj)
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)

# Executable name
set(EXEC XylonEngine)

# Automatically detect all source files (all .cpp files in src and subdirectories)
file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.cpp)

# Exclude files in the lib subdirectory
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

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
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

# Enable Unity Build for faster compilation (CMake 3.16+)
set_target_properties(${EXEC} PROPERTIES UNITY_BUILD ON)

# Optionally, add precompiled headers if you have a common header (e.g., pch.h)
# target_precompile_headers(${EXEC} PRIVATE "${SRC_DIR}/pch.h")

# Link libraries
target_link_libraries(${EXEC} ${LIBS})

# Set output directories
set_target_properties(${EXEC} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# Visual Studio startup project setting (if needed)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXEC})

# Build type specific settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Enable Link Time Optimization and aggressive flags in Release mode
if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -O3 -march=native -flto")
endif()

# Debug flags remain as before
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Clean build directories efficiently
add_custom_target(clean_project
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${OBJ_DIR} ${BIN_DIR}
    COMMENT "Cleaning build directories"
)
