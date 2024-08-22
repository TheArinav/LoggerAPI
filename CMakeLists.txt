cmake_minimum_required(VERSION 3.10)

project(LoggerExample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2 # Replace with the latest version if needed
)

FetchContent_MakeAvailable(json)

# Set build type to Debug
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Ensure proper permissions for temporary files by setting TMPDIR environment variable
set(ENV{TMPDIR} "/tmp")

# Ensure the linker flags are set correctly for debugging
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -pthread")

set(CMAKE_VERBOSE_MAKEFILE ON)

# Check if pscp (part of PuTTY) is available
find_program(PSCP_EXECUTABLE pscp)

if(NOT PSCP_EXECUTABLE)
    message(WARNING "PuTTY's pscp command not found. Attempting to install PuTTY...")

    execute_process(
            COMMAND sudo apt-get install -y putty-tools
            RESULT_VARIABLE APT_GET_RESULT
            OUTPUT_QUIET ERROR_QUIET
    )

    if(APT_GET_RESULT EQUAL 0)
        message(STATUS "PuTTY installed successfully.")
        find_program(PSCP_EXECUTABLE pscp)  # Try to find it again after installation
    else()
        message(FATAL_ERROR "Failed to install PuTTY using apt-get. Please install it manually.")
    endif()

    if(NOT PSCP_EXECUTABLE)
        message(FATAL_ERROR "PuTTY's pscp command still not found after installation attempt. Please ensure PuTTY is installed and pscp is available in your PATH.")
    endif()
else()
    message(STATUS "PuTTY's pscp command found: ${PSCP_EXECUTABLE}")
endif()

# Determine if running under Valgrind
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
    #include <valgrind/valgrind.h>
    int main() {
        return RUNNING_ON_VALGRIND;
    }" RUNNING_ON_VALGRIND)

if(NOT RUNNING_ON_VALGRIND)
    # Enable AddressSanitizer only if not running under Valgrind
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
else()
    message(STATUS "Valgrind detected: AddressSanitizer is disabled to avoid conflicts.")
endif()

# Collect all source files
file(GLOB GENERAL_SRC
        "src/logger_classes/*.hpp"
        "src/logger_classes/*.cpp"
        "src/networking/server/*.hpp"
        "src/networking/server/*.cpp"
        "src/networking/client/*.hpp"
        "src/networking/client/*.cpp"
        "src/networking/transport/*.hpp"
        "src/networking/transport/*.cpp"
        "src/IO/*.hpp"
        "src/IO/*.cpp"
        "src/terminal/*.hpp"
        "src/terminal/*.cpp"
        )

# Add main.cpp explicitly to the source files
set(SOURCE_FILES main.cpp)

# Combine all sources
set(ALL_SOURCES ${SOURCE_FILES} ${GENERAL_SRC})

# Add the executable
add_executable(LoggerExample ${ALL_SOURCES})

#region Dependencies
find_package(Threads REQUIRED)
target_link_libraries(LoggerExample PRIVATE Threads::Threads pthread nlohmann_json::nlohmann_json)

#endregion
