cmake_minimum_required(VERSION 3.12)
project(minihil CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Check for libgpiodcxx (specifically version 2.x or newer)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIODCXX libgpiodcxx>=2.0.0)

# Include directories
include_directories(include)

# Source files (explicitly declared)
set(SOURCES
    src/main.cpp
    src/network/tcp_server.cpp
    src/protocol/jsonrpc_router.cpp
)

if(GPIODCXX_FOUND)
    list(APPEND SOURCES src/hardware/gpiod_relay_controller.cpp)
else()
    list(APPEND SOURCES src/sil/sil_relay_controller.cpp)
endif()

# Executable
add_executable(minihild ${SOURCES})

if(GPIODCXX_FOUND)
    message(STATUS "Found libgpiodcxx >= 2.0.0, building with hardware GPIO support.")
    include_directories(${GPIODCXX_INCLUDE_DIRS})
    target_link_libraries(minihild PRIVATE ${GPIODCXX_LIBRARIES})
    target_compile_definitions(minihild PRIVATE GPIO_HARDWARE_SUPPORT)
else()
    message(WARNING "libgpiodcxx >= 2.0.0 not found. Building in SIL/MOCK mode (software simulation).")
    target_compile_definitions(minihild PRIVATE MOCK_GPIO)
endif()

target_link_libraries(minihild
    PRIVATE
    nlohmann_json::nlohmann_json
)

# Install rules (for Yocto installation)
install(TARGETS minihild DESTINATION bin)
