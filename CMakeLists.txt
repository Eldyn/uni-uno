cmake_minimum_required(VERSION 3.15)
project(ProgettoUno VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(USOCKETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/uSockets/src)

file(GLOB_RECURSE USOCKETS_SOURCES 
    "${USOCKETS_DIR}/*.c"
)

add_library(uSockets STATIC ${USOCKETS_SOURCES})

# WARN: Sta roba non compila con SSL :(
target_compile_definitions(uSockets PUBLIC UWS_NO_ZLIB LIBUS_NO_SSL)

target_include_directories(uSockets PUBLIC ${USOCKETS_DIR})

set(SOURCES 
    src/main.cpp 
)

add_executable(uno_server ${SOURCES})

target_include_directories(uno_server PRIVATE 
    external/uWebSockets/src
    ${USOCKETS_DIR}
    external 
)

# Linking
find_package(SQLite3 REQUIRED)

if(WIN32)
    target_link_libraries(uno_server PRIVATE 
        uSockets 
        SQLite::SQLite3
        iphlpapi psapi user32 crypt32 ws2_32
    )
else()
    target_link_libraries(uno_server PRIVATE 
        uSockets 
        SQLite::SQLite3
        pthread 
        z 
        ssl 
        crypto
    )
endif()

message(STATUS "Configurazione completata per sistema: ${CMAKE_SYSTEM_NAME}")
