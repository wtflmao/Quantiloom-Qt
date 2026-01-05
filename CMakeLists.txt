# ============================================================================
# QuantiloomGUI - Qt6 GUI for Quantiloom Spectral Path Tracer
# ============================================================================

cmake_minimum_required(VERSION 3.21)

project(QuantiloomQt
    VERSION 1.0.0
    DESCRIPTION "Qt6 GUI for libQuantiloom"
    LANGUAGES CXX
)

# ============================================================================
# C++20 Standard
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt6 specific settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Export compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Build Type
# ============================================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# Find Qt6
# ============================================================================
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    LinguistTools
)

message(STATUS "Qt6 version: ${Qt6_VERSION}")
message(STATUS "Qt6 path: ${Qt6_DIR}")

# ============================================================================
# Find Vulkan
# ============================================================================
find_package(Vulkan REQUIRED)
message(STATUS "Vulkan SDK: ${Vulkan_VERSION}")

# ============================================================================
# libQuantiloom Integration
# ============================================================================
# Option 1: Via add_subdirectory (recommended for development)
# Option 2: Via FetchContent
# Option 3: Via find_package (requires installation)

set(QUANTILOOM_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../Quantiloom-dev"
    CACHE PATH "Path to Quantiloom source directory")

if(EXISTS "${QUANTILOOM_SOURCE_DIR}/CMakeLists.txt")
    message(STATUS "Using Quantiloom from: ${QUANTILOOM_SOURCE_DIR}")

    # Disable tests/examples when building as subdirectory
    set(QUANTILOOM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(QUANTILOOM_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    add_subdirectory(${QUANTILOOM_SOURCE_DIR} ${CMAKE_BINARY_DIR}/quantiloom)
else()
    message(FATAL_ERROR
        "Quantiloom source not found at: ${QUANTILOOM_SOURCE_DIR}\n"
        "Please set QUANTILOOM_SOURCE_DIR to the correct path."
    )
endif()

# ============================================================================
# Sources
# ============================================================================
set(GUI_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.hpp
    src/vulkan/QuantiloomVulkanWindow.cpp
    src/vulkan/QuantiloomVulkanWindow.hpp
    src/vulkan/QuantiloomVulkanRenderer.cpp
    src/vulkan/QuantiloomVulkanRenderer.hpp
)

# ============================================================================
# Translations (i18n)
# ============================================================================
set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i18n/translations/quantiloom_zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/src/i18n/translations/quantiloom_en.ts
)

# Generate .qm files from .ts files
qt6_add_translation(QM_FILES ${TS_FILES})

# ============================================================================
# Resources
# ============================================================================
set(RESOURCE_FILES
    resources/quantiloom.qrc
)

# ============================================================================
# Platform Detection (for libQuantiloom headers)
# ============================================================================
if(WIN32)
    set(QUANTILOOM_PLATFORM_WINDOWS 1)
elseif(UNIX AND NOT APPLE)
    set(QUANTILOOM_PLATFORM_LINUX 1)
elseif(APPLE)
    set(QUANTILOOM_PLATFORM_MACOS 1)
endif()

# ============================================================================
# Executable
# ============================================================================
add_executable(QuantiloomQt WIN32
    ${GUI_SOURCES}
    ${QM_FILES}
    ${RESOURCE_FILES}
)

# Platform definition for libQuantiloom headers
if(QUANTILOOM_PLATFORM_WINDOWS)
    target_compile_definitions(QuantiloomQt PRIVATE QUANTILOOM_PLATFORM_WINDOWS)
elseif(QUANTILOOM_PLATFORM_LINUX)
    target_compile_definitions(QuantiloomQt PRIVATE QUANTILOOM_PLATFORM_LINUX)
elseif(QUANTILOOM_PLATFORM_MACOS)
    target_compile_definitions(QuantiloomQt PRIVATE QUANTILOOM_PLATFORM_MACOS)
endif()

# ============================================================================
# Include Directories
# ============================================================================
target_include_directories(QuantiloomQt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================================================
# Link Libraries
# ============================================================================
target_link_libraries(QuantiloomQt PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Vulkan::Vulkan
    libQuantiloom
)

# ============================================================================
# Compiler Flags
# ============================================================================
if(MSVC)
    # UTF-8 source/execution charset
    target_compile_options(QuantiloomQt PRIVATE /utf-8 /W4)
    # Disable specific warnings
    target_compile_options(QuantiloomQt PRIVATE /wd4251)
else()
    target_compile_options(QuantiloomQt PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# Windows-specific: Deploy Qt DLLs
# ============================================================================
if(WIN32)
    # Copy Qt DLLs to output directory (for running from build dir)
    add_custom_command(TARGET QuantiloomQt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE:Qt6::Widgets>
            $<TARGET_FILE:Qt6::Gui>
            $<TARGET_FILE_DIR:QuantiloomQt>
        COMMENT "Copying Qt6 DLLs..."
    )
endif()

# ============================================================================
# Summary
# ============================================================================
message(STATUS "========================================")
message(STATUS "  QuantiloomQt Configuration")
message(STATUS "========================================")
message(STATUS "  Version:      ${PROJECT_VERSION}")
message(STATUS "  Build Type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "  Qt6:          ${Qt6_VERSION}")
message(STATUS "  Vulkan:       ${Vulkan_VERSION}")
message(STATUS "  Quantiloom:   ${QUANTILOOM_SOURCE_DIR}")
message(STATUS "========================================")
