cmake_minimum_required(VERSION 3.15)
project(UniUno VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(libuv REQUIRED)
find_package(uwebsockets REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(jwt-cpp REQUIRED)
find_package(nlohmann_json REQUIRED)

# --- Main Executable ---
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(uno_server ${SOURCES})

# --- Fix for Winsock conflicts on Windows ---
if(WIN32)
    target_compile_definitions(uno_server PRIVATE WIN32_LEAN_AND_MEAN)
endif()

target_include_directories(uno_server PRIVATE include src)

# --- Linking ---
target_link_libraries(uno_server PRIVATE 
    OpenSSL::SSL 
    OpenSSL::Crypto
    ZLIB::ZLIB
    libuv::uv_a
    uwebsockets::uwebsockets
    SQLite::SQLite3
    jwt-cpp::jwt-cpp
    nlohmann_json::nlohmann_json
)

# --- Copy the runtime assets next to the executable ---
# The server resolves the certificates, .env file and public/ folder starting from
# the current working directory. By copying them next to the binary, the executable can
# be launched either from the project root or directly from its own build folder
# (e.g. build/Release/).
#
# Note: the presence of the files is evaluated at configuration time (CMake), so
# the certificates, .env and public/ must be generated BEFORE `cmake --preset`. The setup
# script creates them in the correct order.
set(RUNTIME_FILES cert.pem key.pem .env contract.json)
foreach(asset IN LISTS RUNTIME_FILES)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${asset}")
        add_custom_command(TARGET uno_server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_SOURCE_DIR}/${asset}"
                    "$<TARGET_FILE_DIR:uno_server>/${asset}"
            COMMENT "Copying ${asset} next to the executable")
    else()
        message(WARNING "Runtime asset '${asset}' not found: it will not be copied next to the executable.")
    endif()
endforeach()

if(EXISTS "${CMAKE_SOURCE_DIR}/public")
    add_custom_command(TARGET uno_server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/public"
                "$<TARGET_FILE_DIR:uno_server>/public"
        COMMENT "Copying the public/ folder next to the executable")
else()
    message(WARNING "Folder 'public/' not found: build the frontend first (deploy_frontend.sh).")
endif()

message(STATUS "Configuration complete for system: ${CMAKE_SYSTEM_NAME}")