cmake_minimum_required(VERSION 3.22.1)

project("myapplication")

set(TINYXML2_PATH ${CMAKE_SOURCE_DIR}/tinyxml2)
add_subdirectory(${TINYXML2_PATH} tinyxml2-build)

set(ZLIB_PATH ${CMAKE_SOURCE_DIR}/zlib)
add_subdirectory(${ZLIB_PATH} zlib-build)

set(LIBZIP_PATH ${CMAKE_SOURCE_DIR}/libzip)
add_subdirectory(${LIBZIP_PATH} libzip-build)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}
        tinyxml2
        zlib
        zip
        android
        log)

target_include_directories(${CMAKE_PROJECT_NAME}
        PRIVATE ${TINYXML2_PATH}
        PRIVATE ${ZLIB_PATH}
        PRIVATE ${LIBZIP_PATH}
)
