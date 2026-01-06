# ============================================================================
# QuantiloomGUI - Qt6 GUI for Quantiloom Spectral Path Tracer
# ============================================================================

cmake_minimum_required(VERSION 3.21)

project(QuantiloomQt
    VERSION 0.0.1
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
# libQuantiloom SDK Integration
# ============================================================================
# Uses prebuilt libQuantiloom from SDK directory.
# SDK path auto-detected based on platform (Windows/Linux).
# ============================================================================

set(QUANTILOOM_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../Quantiloom-SDK"
    CACHE PATH "Path to Quantiloom SDK root directory")

# Auto-detect platform-specific SDK path
if(WIN32)
    set(QUANTILOOM_SDK_DIR "${QUANTILOOM_SDK_ROOT}/windows_amd64")
elseif(UNIX AND NOT APPLE)
    set(QUANTILOOM_SDK_DIR "${QUANTILOOM_SDK_ROOT}/linux_amd64")
else()
    message(FATAL_ERROR "Unsupported platform. Only Windows and Linux are supported.")
endif()

if(NOT EXISTS "${QUANTILOOM_SDK_DIR}/lib/cmake/Quantiloom/QuantiloomConfig.cmake")
    message(FATAL_ERROR
        "Quantiloom SDK not found at: ${QUANTILOOM_SDK_DIR}\n"
        "Expected: ${QUANTILOOM_SDK_DIR}/lib/cmake/Quantiloom/QuantiloomConfig.cmake\n"
        "Please set QUANTILOOM_SDK_ROOT to the correct SDK location."
    )
endif()

# Add SDK to CMAKE_PREFIX_PATH for find_package
list(APPEND CMAKE_PREFIX_PATH "${QUANTILOOM_SDK_DIR}")

message(STATUS "Using Quantiloom SDK: ${QUANTILOOM_SDK_DIR}")

# Find prebuilt Quantiloom (also finds Vulkan and glm as dependencies)
find_package(Quantiloom REQUIRED)
message(STATUS "Found Quantiloom: ${Quantiloom_VERSION}")

# Use namespaced target from find_package
set(QUANTILOOM_LIBRARIES Quantiloom::libQuantiloom)

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
# Note: Remove WIN32 for debugging to see console output
# add_executable(QuantiloomQt WIN32
add_executable(QuantiloomQt
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
    ${QUANTILOOM_LIBRARIES}
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
# Windows-specific: Deploy DLLs
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

    # Copy Qt platform plugin (qwindows.dll)
    add_custom_command(TARGET QuantiloomQt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:QuantiloomQt>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::QWindowsIntegrationPlugin>"
            "$<TARGET_FILE_DIR:QuantiloomQt>/platforms/"
        COMMENT "Copying Qt6 platform plugin..."
    )

    # Copy libQuantiloom DLL from SDK
    add_custom_command(TARGET QuantiloomQt POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Quantiloom::libQuantiloom>
            $<TARGET_FILE_DIR:QuantiloomQt>
        COMMENT "Copying libQuantiloom DLL from SDK..."
    )

    # Copy shader files from SDK (required at runtime)
    file(GLOB QUANTILOOM_SHADERS "${QUANTILOOM_SDK_DIR}/bin/*.spv")
    if(QUANTILOOM_SHADERS)
        add_custom_command(TARGET QuantiloomQt POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${QUANTILOOM_SHADERS}
                $<TARGET_FILE_DIR:QuantiloomQt>
            COMMENT "Copying shader files from SDK..."
        )
    endif()
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
message(STATUS "  Quantiloom:   SDK (${QUANTILOOM_SDK_DIR})")
message(STATUS "========================================")
