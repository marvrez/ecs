add_library(${CMAKE_PROJECT_NAME} STATIC
    ecs.h
    common.h
    component.h
    entity.cpp
    entity.h
    registry.cpp
    registry.h
    system.h
)

find_package(Threads REQUIRED)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Werror)
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Threads::Threads)
