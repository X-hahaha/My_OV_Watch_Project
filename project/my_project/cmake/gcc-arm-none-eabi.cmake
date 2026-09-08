set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

# Some default GCC settings
# arm-none-eabi- must be part of path environment

find_program(CMAKE_C_COMPILER
    NAMES arm-none-eabi-gcc
    REQUIRED
)

find_program(CMAKE_ASM_COMPILER
    NAMES arm-none-eabi-gcc
    REQUIRED
)

find_program(CMAKE_OBJCOPY
    NAMES arm-none-eabi-objcopy
    REQUIRED
)

find_program(CMAKE_SIZE
    NAMES arm-none-eabi-size
    REQUIRED
)


#set(TOOLCHAIN_PREFIX                arm-none-eabi-)
#set(CMAKE_C_COMPILER                "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}gcc.exe")
#set(CMAKE_ASM_COMPILER              "${CMAKE_C_COMPILER}")
#set(CMAKE_CXX_COMPILER              "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}g++.exe")
#set(CMAKE_LINKER                    "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}g++.exe")
#set(CMAKE_OBJCOPY                   "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}objcopy.exe")
#set(CMAKE_SIZE                      "${TOOLCHAIN_PATH}/bin/${TOOLCHAIN_PREFIX}size.exe")

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MCU specific flags
set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb")

set(CMAKE_C_FLAGS_INIT
    "${TARGET_FLAGS} -Wall -ffunction-sections -fdata-sections"
)

set(CMAKE_ASM_FLAGS_INIT
    "${TARGET_FLAGS} -x assembler-with-cpp -MMD -MP"
)

set(CMAKE_EXE_LINKER_FLAGS_INIT
    "${TARGET_FLAGS}"
)

set(CMAKE_C_FLAGS_DEBUG_INIT "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE_INIT "-Os -g0")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
#set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

#set(CMAKE_C_FLAGS_DEBUG "-Og -g3")
#set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
#set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3")
#set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")



set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/linker/STM32F411CEU6_APP.ld\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")
set(TOOLCHAIN_LINK_LIBRARIES "m")
