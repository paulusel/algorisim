cmake_minimum_required(VERSION 4.0)

project(algorisim DESCRIPTION "A simulator for algorithms")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Threads REQUIRED)

set(SRC_DIR "src")
file(GLOB_RECURSE SRCFILES "${SRC_DIR}/*.c")

add_executable(algorisim ${SRCFILES})
target_link_libraries(algorisim PRIVATE Threads::Threads)
