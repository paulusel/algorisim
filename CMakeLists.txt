cmake_minimum_required(VERSION 4.0)

project(algorisim DESCRIPTION "A simulator for algorithms")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_subdirectory("modules/dstructures")

find_package(Threads REQUIRED)

set(SRC_DIR "src")
file(GLOB_RECURSE SRCFILES "${SRC_DIR}/*.c")

add_executable(algorisim ${SRCFILES})
target_link_libraries(algorisim PRIVATE Threads::Threads dstruct)

option(USE_ASAN "Use Address Sanitizer." OFF)
if (USE_ASAN)
        message (STATUS "Using Address Sanitizer")
        set_target_properties(algorisim PROPERTIES COMPILE_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
        if (CMAKE_COMPILER_IS_GNUCXX)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
        endif ()
endif (USE_ASAN)
