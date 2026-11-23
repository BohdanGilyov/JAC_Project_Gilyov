cmake_minimum_required(VERSION 3.16)

project(JAC_Project_Gilyov LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(DungeonEscape
    main.c
    game.c
    map.c
    combat.c
    save.c
    game.h
)

if(MSVC)
    target_compile_options(DungeonEscape PRIVATE /W4)
else()
    target_compile_options(DungeonEscape PRIVATE -Wall -Wextra -Wpedantic)
endif()
