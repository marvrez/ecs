cmake_minimum_required(VERSION 3.12)

project(ecs)

add_definitions(-Wno-ignored-attributes)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Output directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)   
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

option(ENABLE_TESTING "Enable unit testing" ON)
if (ENABLE_TESTING)
    include_directories(thirdparty/Catch2/single_include)
    add_subdirectory(tests)
endif()

# Thirdparty libraries
set(TF_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/taskflow)
