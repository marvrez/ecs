add_library(${CMAKE_PROJECT_NAME} STATIC
    ecs.h
    system.h
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Werror)
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)
