cmake_minimum_required(VERSION 3.20)

project(TinyEngine LANGUAGES CXX)

include(FetchContent)

# These values may be supplied by sourcing .env before configuring CMake.
if(NOT DEFINED LLAMA_CPP_ROOT AND DEFINED ENV{LLAMA_CPP_ROOT})
    set(LLAMA_CPP_ROOT "$ENV{LLAMA_CPP_ROOT}" CACHE PATH
        "Homebrew prefix or installation root for llama.cpp")
endif()

if(NOT DEFINED LLAMA_SERVER_BIN AND DEFINED ENV{LLAMA_SERVER_BIN})
    set(LLAMA_SERVER_BIN "$ENV{LLAMA_SERVER_BIN}" CACHE FILEPATH
        "Path to the llama.cpp server executable")
endif()

if(NOT DEFINED LLAMA_CPP_EXPECTED_VERSION AND DEFINED ENV{LLAMA_CPP_EXPECTED_VERSION})
    set(LLAMA_CPP_EXPECTED_VERSION "$ENV{LLAMA_CPP_EXPECTED_VERSION}" CACHE STRING
        "Expected llama.cpp version, if one is required")
endif()

if(NOT DEFINED ABSL_ROOT AND DEFINED ENV{ABSL_ROOT})
    set(ABSL_ROOT "$ENV{ABSL_ROOT}" CACHE PATH
        "Installation root for Abseil")
endif()

set(LLAMA_CPP_ROOT "${LLAMA_CPP_ROOT}" CACHE PATH
    "Homebrew prefix or installation root for llama.cpp")
set(LLAMA_SERVER_BIN "${LLAMA_SERVER_BIN}" CACHE FILEPATH
    "Path to the llama.cpp server executable")
set(LLAMA_CPP_EXPECTED_VERSION "${LLAMA_CPP_EXPECTED_VERSION}" CACHE STRING
    "Expected llama.cpp version, if one is required")
set(ABSL_ROOT "${ABSL_ROOT}" CACHE PATH
    "Installation root for Abseil")

# CMake caches a failed find as LLAMA_SERVER_BIN-NOTFOUND. Clear that value so
# installing llama.cpp later does not require manually deleting the build tree.
if(LLAMA_SERVER_BIN STREQUAL "" OR LLAMA_SERVER_BIN MATCHES "-NOTFOUND$")
    unset(LLAMA_SERVER_BIN CACHE)
    unset(LLAMA_SERVER_BIN)
endif()

add_library(tinyengine_dependencies INTERFACE)
add_library(tinyengine::dependencies ALIAS tinyengine_dependencies)

find_package(CURL REQUIRED)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

target_link_libraries(tinyengine_dependencies INTERFACE CURL::libcurl)

add_executable(tinyengine_cli src/tinyengine_cli.cpp)
target_compile_features(tinyengine_cli PRIVATE cxx_std_17)
target_include_directories(tinyengine_cli PRIVATE include)
target_link_libraries(tinyengine_cli PRIVATE
    tinyengine::dependencies
    nlohmann_json::nlohmann_json
)

if(NOT LLAMA_SERVER_BIN)
    find_program(LLAMA_SERVER_BIN
        NAMES llama-server llama
        HINTS
            "${LLAMA_CPP_ROOT}/bin"
            /opt/homebrew/bin
            /usr/local/bin
    )
endif()

if(LLAMA_SERVER_BIN)
    execute_process(
        COMMAND "${LLAMA_SERVER_BIN}" --version
        OUTPUT_VARIABLE LLAMA_CPP_VERSION_OUTPUT
        ERROR_VARIABLE LLAMA_CPP_VERSION_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "llama.cpp executable: ${LLAMA_SERVER_BIN}")
    message(STATUS "llama.cpp version: ${LLAMA_CPP_VERSION_OUTPUT}")

    if(LLAMA_CPP_EXPECTED_VERSION AND
       NOT LLAMA_CPP_VERSION_OUTPUT MATCHES "${LLAMA_CPP_EXPECTED_VERSION}")
        message(FATAL_ERROR
            "llama.cpp version mismatch. Expected '${LLAMA_CPP_EXPECTED_VERSION}', "
            "got '${LLAMA_CPP_VERSION_OUTPUT}'")
    endif()
else()
    message(WARNING
        "llama.cpp was not found. Install it with 'brew install llama.cpp' "
        "or set LLAMA_SERVER_BIN to its executable path.")
endif()
