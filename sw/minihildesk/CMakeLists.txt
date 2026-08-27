cmake_minimum_required(VERSION 3.12)
project(minihildesk VERSION 1.1.6 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find gtkmm-4.0
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

# Find nlohmann_json
find_package(nlohmann_json REQUIRED)

# Include directories are configured per-target below.

# Find glib-compile-resources
find_program(GLIB_COMPILE_RESOURCES glib-compile-resources REQUIRED)

# Custom command to compile XML resources to resources.cc
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources.cc
    COMMAND ${GLIB_COMPILE_RESOURCES}
            --target=${CMAKE_CURRENT_BINARY_DIR}/resources.cc
            --generate-source
            --sourcedir=${CMAKE_CURRENT_SOURCE_DIR}/resources
            ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.gresource.xml
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.gresource.xml
            ${CMAKE_CURRENT_SOURCE_DIR}/resources/style.css
)

# Source files
file(GLOB_RECURSE SOURCES
    "src/*.cc"
)
list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/resources.cc)

add_executable(minihildesk ${SOURCES})

# Target-specific include directories
target_include_directories(minihildesk PRIVATE
    src
    ${GTKMM_INCLUDE_DIRS}
)

target_compile_options(minihildesk PRIVATE
    -Wall
    -Wextra
    -pedantic
)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

target_link_libraries(minihildesk
    PRIVATE
    ${GTKMM_LIBRARIES}
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
)
