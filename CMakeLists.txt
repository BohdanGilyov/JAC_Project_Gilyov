cmake_minimum_required(VERSION 3.20)
project(JAC_Project_Gilyov C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(GAME_SOURCES game.c map.c combat.c save.c input.c ui.c inventory.c skills.c dialogue.c)
add_executable(DungeonEscape main.c ${GAME_SOURCES})

# Tests are separate from the game.
enable_testing()
add_executable(DungeonEscapeTests tests/test_game.c ${GAME_SOURCES})
add_executable(DungeonEscapeFeatureTests tests/test_features.c ${GAME_SOURCES})
target_include_directories(DungeonEscapeTests PRIVATE .)
target_include_directories(DungeonEscapeFeatureTests PRIVATE .)
add_test(NAME Game COMMAND DungeonEscapeTests)
add_test(NAME Features COMMAND DungeonEscapeFeatureTests)
