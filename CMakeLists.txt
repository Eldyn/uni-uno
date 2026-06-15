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

# --- Fix per i conflitti di Winsock su Windows ---
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

# --- Copia degli asset di runtime accanto all'eseguibile ---
# Il server risolve certificati, file .env e cartella public/ a partire dalla
# directory di lavoro corrente. Copiandoli accanto al binario, l'eseguibile può
# essere avviato indifferentemente dalla root del progetto oppure direttamente
# dalla propria cartella di build (es. build/Release/).
#
# Nota: la presenza dei file è valutata in fase di configurazione (CMake), quindi
# certificati, .env e public/ vanno generati PRIMA di `cmake --preset`. Lo script
# di setup li crea nell'ordine corretto.
set(RUNTIME_FILES cert.pem key.pem .env)
foreach(asset IN LISTS RUNTIME_FILES)
    if(EXISTS "${CMAKE_SOURCE_DIR}/${asset}")
        add_custom_command(TARGET uno_server POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_SOURCE_DIR}/${asset}"
                    "$<TARGET_FILE_DIR:uno_server>/${asset}"
            COMMENT "Copia ${asset} accanto all'eseguibile")
    else()
        message(WARNING "Asset di runtime '${asset}' non trovato: non verrà copiato accanto all'eseguibile.")
    endif()
endforeach()

if(EXISTS "${CMAKE_SOURCE_DIR}/public")
    add_custom_command(TARGET uno_server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/public"
                "$<TARGET_FILE_DIR:uno_server>/public"
        COMMENT "Copia la cartella public/ accanto all'eseguibile")
else()
    message(WARNING "Cartella 'public/' non trovata: compila prima il frontend (deploy_frontend.sh).")
endif()

message(STATUS "Configuration complete for system: ${CMAKE_SYSTEM_NAME}")