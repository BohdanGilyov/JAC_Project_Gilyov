#include "game.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_catalog_inventory_and_store(void)
{
    GameState game;
    init_game(&game);
    assert(game.gold == 70);
    for (int i = 0; i < ITEM_COUNT; i++) {
        assert(item_catalog[i].type == (ItemType)(i / ITEM_PER_TYPE));
        assert(item_catalog[i].price > 0 && item_catalog[i].bonus > 0);
    }
    assert(game.inventory[10] == 2);
    assert(!item_buy(&game, -1));
    assert(item_buy(&game, 0));
    assert(game.gold == 50);
    assert(game.inventory[0] == 1);
    (void)item_equip_or_use(&game, 0);
    assert(game.equipped_weapon == 0);
    assert(effective_attack(&game) == 14);

    assert(item_buy(&game, 5));
    assert(game.gold == 30);
    (void)item_equip_or_use(&game, 5);
    assert(effective_defense(&game) == 4);
    assert(!item_buy(&game, 14)); /* nedostatok zlata */
    assert(!item_sell(&game, 12));

    assert(item_sell(&game, 0));
    assert(game.gold == 40);
    assert(game.inventory[0] == 0);
    assert(game.equipped_weapon == -1);
    assert(effective_attack(&game) == 12);

    game.player.hp = 92;
    assert(item_equip_or_use(&game, 10));
    assert(game.player.hp == 100);
    assert(game.inventory[10] == 1);
    assert(!item_equip_or_use(&game, 10));
    assert(game.inventory[10] == 1);
    assert(item_sell(&game, 5));
    assert(game.equipped_armor == -1);
}

static void test_five_skills(void)
{
    GameState game;
    Enemy enemy;
    int block = 0;
    char message[160];
    init_game(&game);
    enemy = game.enemies[4];
    enemy.hp = 400;
    enemy.max_hp = 400;
    assert(ability_unlocked(&game, 0));
    for (int i = 1; i < 5; i++) assert(!ability_unlocked(&game, i));
    assert(use_ability(&game, &enemy, 0, &block, message, sizeof(message)));
    assert(enemy.hp < 400);
    assert(game.mana == 17);
    assert(!use_ability(&game, &enemy, 4, &block, message, sizeof(message)));

    gain_xp(&game, 200);
    assert(game.player.level == 5 && game.mana == 40);
    for (int i = 0; i < 5; i++) {
        assert(ability_unlocked(&game, i));
        assert(ability_cost(i) > 0);
    }
    game.player.hp = 60;
    assert(use_ability(&game, &enemy, 1, &block, message, sizeof(message)));
    assert(game.player.hp == 90);
    assert(use_ability(&game, &enemy, 2, &block, message, sizeof(message)));
    assert(block == 1);
    int old_hp = enemy.hp;
    assert(use_ability(&game, &enemy, 3, &block, message, sizeof(message)));
    assert(enemy.hp == old_hp - 30);
    old_hp = enemy.hp;
    assert(use_ability(&game, &enemy, 4, &block, message, sizeof(message)));
    assert(enemy.hp < old_hp);
    game.mana = 0;
    assert(!use_ability(&game, &enemy, 0, &block, message, sizeof(message)));
}

static void test_five_dialogue_endings(void)
{
    GameState game;
    init_game(&game);
    game.player.hp = 25;
    assert(dialogue_finish(&game, 1));
    assert(game.player.hp == 100);
    int gold = game.gold;
    assert(dialogue_finish(&game, 2));
    assert(game.gold == gold + 25);
    assert(dialogue_finish(&game, 2));
    assert(game.gold == gold + 25); /* odmenu nedostane dvakrat */
    assert(dialogue_finish(&game, 3));
    assert(game.player.xp == 20);
    assert(game.player.level == 2);
    assert(dialogue_finish(&game, 4));
    assert(game.inventory[13] == 1);
    assert(dialogue_finish(&game, 5));
    assert(game.gold == gold + 15);
    assert(game.dialogue_last == 5);
    for (int i = 0; i < DIALOG_ENDINGS; i++) assert(game.dialogue_seen[i] == 1);
    assert(!dialogue_finish(&game, 0));
    assert(!dialogue_finish(&game, 6));
}

static void test_new_save_and_old_save_compatibility(void)
{
    const char *path = "test_feature_save.txt";
    GameState before;
    GameState after;
    init_game(&before);
    assert(item_buy(&before, 0));
    (void)item_equip_or_use(&before, 0);
    assert(dialogue_finish(&before, 2));
    assert(dialogue_finish(&before, 4));
    gain_xp(&before, 60);
    before.player.row = 1;
    before.player.col = 2;
    assert(map_tile(before.player.row, before.player.col) == 'M');
    assert(map_tile(2, 3) == 'N');
    assert(save_game(&before, path));
    assert(load_game(&after, path));
    assert(memcmp(&before.player, &after.player, sizeof(Player)) == 0);
    assert(memcmp(before.inventory, after.inventory, sizeof(before.inventory)) == 0);
    assert(memcmp(before.dialogue_seen, after.dialogue_seen,
                  sizeof(before.dialogue_seen)) == 0);
    assert(after.gold == before.gold);
    assert(after.mana == before.mana && after.max_mana == before.max_mana);
    assert(after.equipped_weapon == before.equipped_weapon);
    assert(after.dialogue_last == before.dialogue_last);
    assert(remove(path) == 0);

    FILE *file = fopen(path, "w");
    assert(file);
    assert(fputs("DUNGEON_ESCAPE_V1\n"
                 "player 1 1 100 100 12 3 1 0\n"
                 "progress 0 0 0 0 0 0\n"
                 "enemies 1 1 1 1 1\n", file) >= 0);
    assert(fclose(file) == 0);
    assert(load_game(&after, path));
    assert(after.gold == 70 && after.inventory[10] == 2);
    assert(after.mana == 20 && after.max_mana == 20);
    assert(after.equipped_weapon == -1 && after.equipped_armor == -1);
    assert(remove(path) == 0);
}

int main(void)
{
    test_catalog_inventory_and_store();
    test_five_skills();
    test_five_dialogue_endings();
    test_new_save_and_old_save_compatibility();
    puts("OK: 15 predmetov, obchod, vybava, 5 schopnosti, 5 koncov dialogu, save/load V1/V2.");
    return 0;
}
