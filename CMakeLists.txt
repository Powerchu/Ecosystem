cmake_minimum_required(VERSION 3.16)
project(EcosystemProject VERSION 1.0.0 LANGUAGES CXX C)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(OpenGL REQUIRED)

# Option to use system GLFW or bundled version
option(USE_SYSTEM_GLFW "Use system-installed GLFW" ON)

if(USE_SYSTEM_GLFW)
    find_package(glfw3 REQUIRED)
else()
    # Add GLFW subdirectory if building from source
    # This would require downloading GLFW source
    message(STATUS "Using bundled GLFW - please ensure GLFW source is available")
endif()

# Create executable
add_executable(EcosystemProject
    # Main source
    Ecosystem_Project/Source/main.cpp
    
    # Ecosystem sources
    Ecosystem_Project/Source/EcoSystem/EcoSystem.cpp
    Ecosystem_Project/Source/EcoSystem/Terrain.cpp
    
    # Tools
    Ecosystem_Project/Source/EcoSystem/Tools/LogTool.cpp
    Ecosystem_Project/Source/EcoSystem/Tools/SpawnTool.cpp
    Ecosystem_Project/Source/EcoSystem/Tools/Tools.cpp
    Ecosystem_Project/Source/EcoSystem/Tools/ViewerTool.cpp
    
    # Creatures
    Ecosystem_Project/Source/Creatures/Creature.cpp
    Ecosystem_Project/Source/Creatures/Fox.cpp
    Ecosystem_Project/Source/Creatures/Rabbit.cpp
    
    # Data
    Ecosystem_Project/Source/Data/EcoData.cpp
    
    # ImGui sources
    Ecosystem_Project/Dependencies/imgui.cpp
    Ecosystem_Project/Dependencies/imgui_demo.cpp
    Ecosystem_Project/Dependencies/imgui_draw.cpp
    Ecosystem_Project/Dependencies/imgui_widgets.cpp
    Ecosystem_Project/Dependencies/imgui_impl_glfw.cpp
    Ecosystem_Project/Dependencies/imgui_impl_opengl3.cpp
    
    # GL3W
    Ecosystem_Project/Dependencies/libs/gl3w/GL/gl3w.c
)

# Include directories
target_include_directories(EcosystemProject PRIVATE
    Ecosystem_Project/Include
    Ecosystem_Project/Dependencies
    Ecosystem_Project/Dependencies/libs/gl3w
    Ecosystem_Project/Dependencies/libs/glfw/include
)

# Link libraries
if(USE_SYSTEM_GLFW)
    target_link_libraries(EcosystemProject PRIVATE glfw)
else()
    target_link_libraries(EcosystemProject PRIVATE glfw3)
endif()

target_link_libraries(EcosystemProject PRIVATE OpenGL::GL)

# Platform-specific settings
if(APPLE)
    # macOS frameworks
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    
    target_link_libraries(EcosystemProject PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
    
    # Set minimum macOS version
    set_target_properties(EcosystemProject PROPERTIES
        MACOSX_DEPLOYMENT_TARGET "10.12"
    )
    
elseif(WIN32)
    # Windows-specific libraries
    target_link_libraries(EcosystemProject PRIVATE
        legacy_stdio_definitions
    )
    
elseif(UNIX AND NOT APPLE)
    # Linux-specific libraries
    target_link_libraries(EcosystemProject PRIVATE
        dl
        pthread
        X11
        Xrandr
        Xinerama
        Xcursor
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(EcosystemProject PRIVATE /W4)
    target_compile_definitions(EcosystemProject PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(EcosystemProject PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy assets if any exist
if(EXISTS "${CMAKE_SOURCE_DIR}/Ecosystem_Project/imgui.ini")
    configure_file(
        "${CMAKE_SOURCE_DIR}/Ecosystem_Project/imgui.ini"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/imgui.ini"
        COPYONLY
    )
endif()

# Print build information
message(STATUS "Building EcosystemProject")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Using system GLFW: ${USE_SYSTEM_GLFW}")