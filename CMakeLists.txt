cmake_minimum_required(VERSION 3.15)
project(UniUno VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(USOCKETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/uWebSockets/uSockets/src)

file(GLOB_RECURSE USOCKETS_SOURCES 
    "${USOCKETS_DIR}/*.c"
    "${USOCKETS_DIR}/crypto/*.c"
    "${USOCKETS_DIR}/crypto/*.cpp"
)

add_library(uSockets STATIC ${USOCKETS_SOURCES})
set_target_properties(uSockets PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(uSockets PUBLIC ${USOCKETS_DIR})
target_compile_definitions(uSockets PUBLIC LIBUS_USE_OPENSSL)

# SQLite amalgamation
add_library(sqlite3 STATIC
    external/sqlite3/sqlite3.c
)
target_include_directories(sqlite3 PUBLIC
    external/sqlite3
)
target_compile_options(sqlite3 PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/w>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-w>
)
target_compile_definitions(sqlite3 PUBLIC
    SQLITE_OMIT_LOAD_EXTENSION
    SQLITE_THREADSAFE=1
)

set(SOURCES 
    src/main.cpp 
    src/WebServer.cpp
)

add_executable(uno_server ${SOURCES})

target_include_directories(uno_server PRIVATE 
    external/uWebSockets/src
    ${USOCKETS_DIR}
    external 
)

# Linking
find_package(OpenSSL REQUIRED)

if(WIN32)
    target_link_libraries(uno_server PRIVATE 
        uSockets 
        sqlite3
        OpenSSL::SSL OpenSSL::Crypto
        iphlpapi psapi user32 crypt32 ws2_32
    )
else()
    target_link_libraries(uno_server PRIVATE 
        uSockets 
        sqlite3
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread 
        z 
    )
endif()

message(STATUS "Configurazione completata per sistema: ${CMAKE_SYSTEM_NAME}")
