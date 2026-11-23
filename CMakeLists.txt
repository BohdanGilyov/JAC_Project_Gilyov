
cmake_minimum_required(VERSION 3.20)

project(JAC_Project_Gilyov C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(DungeonEscape
        main.c
        game.c
        map.c
        combat.c
        save.c
        input.c
        ui.c
        inventory.c
        skills.c
        dialogue.c
)

if(WIN32)
    target_link_libraries(DungeonEscape PRIVATE user32)
endif()
