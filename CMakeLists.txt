cmake_minimum_required(VERSION 3.10)
project(csynth C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /WX)
    set(CMAKE_C_FLAGS_DEBUG "/Od /Zi /MDd")
    set(CMAKE_C_FLAGS_RELEASE "/O2 /DNDEBUG /MD")
else()
    add_compile_options(-Wall -Werror)
    set(CMAKE_C_FLAGS_DEBUG "-g -O0")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

find_package(SDL3 CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 $<$<NOT:$<BOOL:${MSVC}>>:m>)
