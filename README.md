# ecs
![Build](https://github.com/marvrez/ecs/workflows/Build/badge.svg)

Minimalistic entity-component system with support for multi-threaded execution of systems.

## Why though?
Most other ECS implementations I found seemed to be quite overkill for what I actually needed.
Moreover, many of them had APIs that I didn't find very intuitive. For that reason, I decided to create my own ECS that is minimal and simple to understand.

## Build
Before you can build, you need to make sure you have [CMake](https://cmake.org/download/) 3.12 or newer.
In addition, you need to use a C++17 compliant compiler or newer – most modern compilers should already have that sorted for you.

You can clone and build the project by executing the following:

```sh
git clone --recursive https://github.com/marvrez/ecs.git
cd ecs
mkdir build && cd build
cmake ..
cmake --build . -j4
```
The tests can be executed by running:
```sh
./bin/ecs-tests
```
You can omit building the tests by passing `-DENABLE_TESTING=OFF` to cmake.
