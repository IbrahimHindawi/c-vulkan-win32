cmake_minimum_required(VERSION 3.10)

project(c-vulkan-win32 VERSION 1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_C_STANDARD 99)
# set(CMAKE_C_STANDARD_REQUIRED True)

find_package(Vulkan REQUIRED)

include_directories(extern/haikal/src/meta_arena/gen)
include_directories(extern/saha/src)
include_directories($ENV{VULKAN_SDK}/Include)

# target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

add_executable(${PROJECT_NAME}
    src/main.c
)
# add_subdirectory(extern) # in case of modules...

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
endif()

# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resource ${CMAKE_CURRENT_BINARY_DIR}/resource)
