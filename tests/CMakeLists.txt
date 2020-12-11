add_executable(ecs-tests tests.cpp)

target_compile_features(ecs-tests PRIVATE cxx_std_17)
target_compile_options(ecs-tests PRIVATE -Wall -Werror)
target_include_directories(ecs-tests PRIVATE ${PROJECT_SOURCE_DIR})
