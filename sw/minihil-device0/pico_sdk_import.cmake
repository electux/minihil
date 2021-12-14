#
# @brief   minihil RPI Pico SDK import
# @version v1.0.3
# @date    2021-12-15
# @author  Vladimir Roncevic <elektron.ronca@gmail.com>
#

# Checking SDK path
if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    message("Using PICO_SDK_PATH from environment ('${PICO_SDK_PATH}')")
endif ()

# Checking SDK path fetched from GIT
if (DEFINED ENV{PICO_SDK_FETCH_FROM_GIT} AND (NOT PICO_SDK_FETCH_FROM_GIT))
    set(PICO_SDK_FETCH_FROM_GIT $ENV{PICO_SDK_FETCH_FROM_GIT})
    message(
        "Using PICO_SDK_FETCH_FROM_GIT"
        "from environment ('${PICO_SDK_FETCH_FROM_GIT}')"
    )
endif ()

if (DEFINED ENV{PICO_SDK_FETCH_FROM_GIT_PATH} AND (NOT PICO_SDK_FETCH_FROM_GIT_PATH))
    set(PICO_SDK_FETCH_FROM_GIT_PATH $ENV{PICO_SDK_FETCH_FROM_GIT_PATH})
    message(
        "Using PICO_SDK_FETCH_FROM_GIT_PATH"
        "from environment ('${PICO_SDK_FETCH_FROM_GIT_PATH}')"
    )
endif ()

# Setup RPI PICO SDK path
set(
    PICO_SDK_PATH "${PICO_SDK_PATH}" CACHE PATH "Path to the RPI Pi Pico SDK"
)
set(
    PICO_SDK_FETCH_FROM_GIT "${PICO_SDK_FETCH_FROM_GIT}"
    CACHE BOOL
    "Set to ON to fetch copy of SDK from git if not otherwise locatable"
)
set(
    PICO_SDK_FETCH_FROM_GIT_PATH "${PICO_SDK_FETCH_FROM_GIT_PATH}"
    CACHE FILEPATH "location to download SDK"
)

# Fetch RPI PI SDK from GitHub
if (NOT PICO_SDK_PATH)
    if (PICO_SDK_FETCH_FROM_GIT)
        include(FetchContent)
        set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
        if (PICO_SDK_FETCH_FROM_GIT_PATH)
            get_filename_component(
                FETCHCONTENT_BASE_DIR "${PICO_SDK_FETCH_FROM_GIT_PATH}"
                REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}"
            )
        endif ()
        FetchContent_Declare(
            pico_sdk
            GIT_REPOSITORY https://github.com/RPIpi/pico-sdk
            GIT_TAG master
        )
        if (NOT pico_sdk)
            message("Downloading RPI Pi Pico SDK")
            FetchContent_Populate(pico_sdk)
            set(PICO_SDK_PATH ${pico_sdk_SOURCE_DIR})
        endif ()
        set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})
    else ()
        message(
            FATAL_ERROR
            "SDK location was not specified."
            "Please set PICO_SDK_PATH or set PICO_SDK_FETCH_FROM_GIT."
        )
    endif ()
endif ()

get_filename_component(
    PICO_SDK_PATH "${PICO_SDK_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}"
)
if (NOT EXISTS ${PICO_SDK_PATH})
    message(FATAL_ERROR "Directory '${PICO_SDK_PATH}' not found")
endif ()

set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)

# Check RPI PI SDK init configuration
if (NOT EXISTS ${PICO_SDK_INIT_CMAKE_FILE})
    message(
        FATAL_ERROR
        "Dir '${PICO_SDK_PATH}' does not appear to contain RPI Pi Pico SDK"
    )
endif ()

set(
    PICO_SDK_PATH ${PICO_SDK_PATH} CACHE PATH "Path to RPI Pi Pico SDK" FORCE
)

include(${PICO_SDK_INIT_CMAKE_FILE})
