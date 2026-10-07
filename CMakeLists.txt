cmake_minimum_required(VERSION 3.21)
project(FrameWindowCounter VERSION 1.2.6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    Geode
    GIT_REPOSITORY https://github.com/geode-sdk/geode.git
    GIT_TAG main
)
FetchContent_MakeAvailable(Geode)

file(GLOB_RECURSE SOURCES "src/*.cpp")

setup_geode_mod(
    ${PROJECT_NAME}
    SOURCES ${SOURCES}
)