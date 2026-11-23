#include "game.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_initial_state(void)
{
    GameState game;
    init_game(&game);
    assert(game.player.level == 1);
    assert(game.player.xp == 0);
    assert(game.player.hp == 100);
    assert(game.player.row == 1 && game.player.col == 1);
    assert(MAP_ROWS >= 5 && MAP_COLS >= 5);
    assert(map_tile(-1, 1) == '#');
    assert(map_tile(3, 3) == 'H');
    assert(map_tile(6, 6) == 'X');
    for (int i = 0; i < ENEMY_COUNT; i++) {
        assert(game.enemies[i].alive);
        assert(game.enemies[i].hp > 0);
        assert(game.enemies[i].attack > 0);
        assert(game.enemies[i].defense >= 0);
    }
}

static void test_map_shrine_and_levels(void)
{
    GameState game;
    init_game(&game);
    move_player(&game, -1, 0);
    assert(game.player.row == 1 && game.player.col == 1);
    move_player(&game, 0, 1);
    assert(game.player.col == 2);
    /* Svatyna je dosiahnutelna z pozicie (3, 2). */
    game.player.row = 3;
    game.player.col = 2;
    game.player.hp = 45;
    move_player(&game, 0, 1);
    assert(game.quest_shrine_done == 1);
    assert(game.shrine_used == 1);
    assert(game.player.xp == 20);
    assert(game.player.level == 2);
    assert(game.player.hp == 110);
    assert(game.player.max_hp == 110);
    move_player(&game, 0, -1);
    move_player(&game, 0, 1);
    assert(game.player.xp == 20);
    gain_xp(&game, 200);
    assert(game.player.level == 5);
    assert(game.player.max_hp == 140);
    assert(game.player.attack == 20);
    assert(game.player.defense == 7);
}

static void test_quests_boss_and_exit(void)
{
    GameState game;
    init_game(&game);
    game.player.row = 6;
    game.player.col = 5;
    /* Zamknuty vychod pred splnenim uloh. */
    move_player(&game, 0, 1);
    assert(game.player.col == 5);
    assert(!game.quest_escape_done);
    for (int i = 0; i < 3; i++) {
        on_enemy_defeated(&game, i);
    }
    assert(game.normal_kills == 3);
    assert(game.quest_hunter_done);
    assert(game.player.xp == 15 + 20 + 25 + 35);
    on_enemy_defeated(&game, 4);
    assert(game.boss_defeated);
    assert(game.enemies[4].alive == 0);
    move_player(&game, 0, 1);
    assert(!game.quest_escape_done); /* Este treba navstivit svatynu. */
    game.player.row = 3;
    game.player.col = 2;
    move_player(&game, 0, 1);
    assert(game.quest_shrine_done);
    game.player.row = 6;
    game.player.col = 5;
    move_player(&game, 0, 1);
    assert(game.quest_escape_done);
    assert(game.player.level == MAX_LEVEL);
}

static void test_save_and_load(void)
{
    static const char path[] = "test_save.txt";
    GameState before;
    GameState after;
    init_game(&before);
    before.player.row = 3;
    before.player.col = 2;
    move_player(&before, 0, 1);
    on_enemy_defeated(&before, 0);
    assert(save_game(&before, path));
    init_game(&after);
    assert(load_game(&after, path));
    assert(memcmp(&after.player, &before.player, sizeof(Player)) == 0);
    assert(after.shrine_used == before.shrine_used);
    assert(after.enemies[0].alive == 0);
    assert(after.enemies[1].alive == 1);
    assert(remove(path) == 0);

    /* Neplatny subor sa nesmie nacitat. */
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    assert(fputs("poskodeny subor\n", file) >= 0);
    assert(fclose(file) == 0);
    assert(!load_game(&after, path));
    assert(remove(path) == 0);
}

int main(void)
{
    test_initial_state();
    test_map_shrine_and_levels();
    test_quests_boss_and_exit();
    test_save_and_load();
    puts("OK: mapa, 5 nepriatelov, levely 1-5, 3 ulohy, save/load.");
    return 0;
}
