# Keep manually managed dependencies outside CubeMX's reserved Middlewares folder.
set(FREERTOS_KERNEL_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/FreeRTOS-Kernel
)

set(FREERTOS_REQUIRED_FILES
    tasks.c
    queue.c
    list.c
    timers.c
    event_groups.c
    stream_buffer.c
    include/FreeRTOS.h
    portable/GCC/ARM_CM4F/port.c
    portable/GCC/ARM_CM4F/portmacro.h
    portable/MemMang/heap_4.c
)

foreach(FILE_PATH IN LISTS FREERTOS_REQUIRED_FILES)
    if(NOT EXISTS "${FREERTOS_KERNEL_DIR}/${FILE_PATH}")
        message(FATAL_ERROR
            "FreeRTOS kernel file not found: ${FREERTOS_KERNEL_DIR}/${FILE_PATH}. "
            "From the repository root, run: git submodule update --init --checkout -- project/my_project/ThirdParty/FreeRTOS-Kernel"
        )
    endif()
endforeach()

add_library(freertos_kernel STATIC
    ${FREERTOS_KERNEL_DIR}/tasks.c
    ${FREERTOS_KERNEL_DIR}/queue.c
    ${FREERTOS_KERNEL_DIR}/list.c
    ${FREERTOS_KERNEL_DIR}/timers.c
    ${FREERTOS_KERNEL_DIR}/event_groups.c
    ${FREERTOS_KERNEL_DIR}/stream_buffer.c
    ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM4F/port.c
    ${FREERTOS_KERNEL_DIR}/portable/MemMang/heap_4.c
)

target_include_directories(freertos_kernel PUBLIC
    ${FREERTOS_KERNEL_DIR}/include
    ${FREERTOS_KERNEL_DIR}/portable/GCC/ARM_CM4F
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
)

target_link_libraries(freertos_kernel PUBLIC stm32cubemx)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE APP_USE_FREERTOS=1)
target_link_libraries(${CMAKE_PROJECT_NAME} freertos_kernel)




