set(APP_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/app_entry.c
)
set(APP_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)    



target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${APP_SOURCE}
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${APP_INCLUDE_DIR})
