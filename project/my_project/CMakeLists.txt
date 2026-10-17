cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
#--------------------------------------------------
set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -DDEBUG" CACHE STRING "Debug C flags" FORCE)
set(CMAKE_ASM_FLAGS_DEBUG "-g3" CACHE STRING "Debug ASM flags" FORCE)

# Release 配置：尺寸最优（计划固定标准）
set(CMAKE_C_FLAGS_RELEASE "-Os" CACHE STRING "Release C flags" FORCE)
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE STRING "Release ASM flags" FORCE)
#--------------------------------------------------

# Set the project name
set(CMAKE_PROJECT_NAME my_project)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
add_subdirectory(Diag)
add_subdirectory(APP)
add_subdirectory(BSP)
add_subdirectory(SYSTEM)

# Add FreeRTOS kernel
include(cmake/freertos_kernel.cmake)
# Add LVGL  
include(cmake/lvgl.cmake)

# define USER_VECT_TAB_ADDRESS
target_compile_definitions(STM32_Drivers PRIVATE USER_VECT_TAB_ADDRESS VECT_TAB_OFFSET=0x0000C000U)


# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# ========== 自动生成 HEX/BIN/MAP + 内存统计 ==========
set(ELF_FILE ${CMAKE_PROJECT_NAME}.elf)
set(HEX_FILE ${CMAKE_PROJECT_NAME}.hex)
set(BIN_FILE ${CMAKE_PROJECT_NAME}.bin)
set(MAP_FILE ${CMAKE_PROJECT_NAME}.map)

# 链接后自动执行：生成 HEX、BIN，打印内存占用
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    # 生成 HEX（Intel HEX 格式，带地址和校验）
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${HEX_FILE}
    # 生成 BIN（纯二进制，直接写入 Flash）
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${BIN_FILE}
    # 打印各段大小和内存占用百分比
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    # 打印详细的内存区域占用（需要链接脚本里有 --print-memory-usage）
    COMMENT "=== Build output: ${HEX_FILE}, ${BIN_FILE} ==="
)
