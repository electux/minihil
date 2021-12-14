#
# @brief   minihil setup output files
# @version v1.0.3
# @date    2021-12-15
# @author  Vladimir Roncevic <elektron.ronca@gmail.com>
#

# Create executable
add_executable("${CMAKE_PROJECT_NAME}" main.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries("${CMAKE_PROJECT_NAME}" pico_stdlib)

# Create extra output files map/bin/hex
pico_add_extra_outputs("${CMAKE_PROJECT_NAME}")

# Add url via pico_set_program_url
pro_auto_set_url("${CMAKE_PROJECT_NAME}")

