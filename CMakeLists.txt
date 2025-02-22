cmake_minimum_required(VERSION 3.10)
project(XylonEngine)

# set policies
cmake_policy(SET CMP0072 NEW)

# source, object, and binary directories
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(OBJ_DIR ${CMAKE_BINARY_DIR}/obj)
set(BIN_DIR ${CMAKE_BINARY_DIR}/bin)

# executable name
set(EXEC XylonEngine)

# automatically detect all source files (all .cpp files in src and subdirectories)
file(GLOB_RECURSE SOURCES
    ${SRC_DIR}/*.cpp
)

foreach(TO_REMOVE ${SOURCES})
    if(${TO_REMOVE} MATCHES "${SRC_DIR}/lib/")
        list(REMOVE_ITEM SOURCES ${TO_REMOVE})
    endif()
endforeach()

# find external packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2_ttf REQUIRED)

# libraries to link
set(LIBS
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    SDL2_image
    SDL2_mixer
    SDL2_ttf
)

# create executable
add_executable(${EXEC} ${SOURCES})

# link libraries
target_link_libraries(${EXEC} ${LIBS})

# set output directories
set_target_properties(${EXEC} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

# visual Studio specific settings (if needed)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXEC})

# add Debug/Release build settings
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# clean build directories efficiently
add_custom_target(clean_project
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${OBJ_DIR} ${BIN_DIR}
    COMMENT "Cleaning build directories"
)
