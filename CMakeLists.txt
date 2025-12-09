cmake_minimum_required(VERSION 3.10)
project(FishStory2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Model.cpp
    src/Player.cpp
    src/Ocean.cpp
    src/Game.cpp
    src/Collectible.cpp
    src/Enemy.cpp
    src/TextRenderer.cpp
    src/AudioEngine.cpp  # <--- ADDED THIS
)

set(HEADERS
    src/Shader.h
    src/Camera.h
    src/Model.h
    src/Player.h
    src/Ocean.h
    src/Game.h
    src/Collectible.h
    src/Enemy.h
    src/TextRenderer.h
    src/AudioEngine.h    # <--- ADDED THIS
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)

# GLFW
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

# GLEW
find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)

# GLM
find_path(GLM_INCLUDE_DIR glm/glm.hpp
    HINTS /usr/include /usr/local/include)
if(GLM_INCLUDE_DIR)
    include_directories(${GLM_INCLUDE_DIR})
    message(STATUS "Found GLM: ${GLM_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "GLM not found")
endif()

# ASSIMP
find_package(assimp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} assimp::assimp)

# Copy assets to build directory and to Release/Debug folders for Visual Studio
file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

# For Visual Studio multi-config generators, also copy to config-specific directories
if(MSVC)
    file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR}/Release)
    file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR}/Release)
    file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR}/Debug)
    file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR}/Debug)
endif()

# Copy the new asset folders if they exist
if(EXISTS "${CMAKE_SOURCE_DIR}/audio")
    file(COPY ${CMAKE_SOURCE_DIR}/audio DESTINATION ${CMAKE_BINARY_DIR})
    if(MSVC)
        file(COPY ${CMAKE_SOURCE_DIR}/audio DESTINATION ${CMAKE_BINARY_DIR}/Release)
        file(COPY ${CMAKE_SOURCE_DIR}/audio DESTINATION ${CMAKE_BINARY_DIR}/Debug)
    endif()
endif()
if(EXISTS "${CMAKE_SOURCE_DIR}/textures")
    file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})
    if(MSVC)
        file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR}/Release)
        file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR}/Debug)
    endif()
endif()

# Platform-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE OFF
    )
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings (miniaudio needs dl and pthread)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} pthread)
endif()

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)