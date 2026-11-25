#include "game.h"

#include <stdio.h>
#include <string.h>

static const int level_xp[MAX_LEVEL] = {0, 20, 50, 90, 140};

void init_game(struct GameState *game)
{
    struct GameState initial = {0};
    struct Player player = {1, 1, 100, 100, 12, 3, 1, 0};
    struct Enemy enemies[ENEMY_COUNT] = {
        {"Rat",      2, 6, 15, 15,  5, 1, 15, 1, 0},
        {"Skeleton", 5, 1, 25, 25,  8, 2, 20, 1, 0},
        {"Goblin",   4, 4, 35, 35,  9, 3, 25, 1, 0},
        {"Orc",      5, 6, 45, 45, 11, 4, 30, 1, 0},
        {"Boss",     6, 5, 70, 70, 14, 5, 60, 1, 1}
    };
    *game = initial;
    game->player = player;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        game->enemies[i] = enemies[i];
    }
    game->gold = 70;
    game->mana = 20;
    game->max_mana = 20;
    game->equipped_weapon = -1;
    game->equipped_armor = -1;
    game->inventory[10] = 2; /* 2 male elixiry na zaciatok */
    snprintf(game->message, sizeof(game->message), "Najdi svatynu H, obchod M, mudrca N a unikni cez X!");
}

void gain_xp(struct GameState *game, int amount)
{
    struct Player *p = &game->player;
    int old_level = p->level;
    p->xp += amount;

    while (p->level < MAX_LEVEL && p->xp >= level_xp[p->level]) {
        p->level++;
        p->max_hp += 10;
        p->attack += 2;
        p->defense++;
        game->max_mana += 5;
        game->mana = game->max_mana;
    }

    if (p->level > old_level) {
        snprintf(game->message, sizeof(game->message), "+%d XP! LEVEL UP: %d -> %d. Max HP zvysene!",
                     amount, old_level, p->level);
    } else {
        snprintf(game->message, sizeof(game->message), "Ziskal si %d XP (celkovo %d).", amount, p->xp);
    }
}

void on_enemy_defeated(struct GameState *game, int enemy_index)
{
    struct Enemy *enemy;
    int old_level;
    int reward;
    int quest_reward = 0;

    if (enemy_index < 0 || enemy_index >= ENEMY_COUNT) {
        return;
    }
    enemy = &game->enemies[enemy_index];
    if (!enemy->alive) {
        return;
    }

    enemy->alive = 0;
    enemy->hp = 0;
    old_level = game->player.level;
    reward = enemy->xp_reward;

    if (enemy->is_boss) {
        game->boss_defeated = 1;
    } else {
        game->normal_kills++;
    }

    gain_xp(game, reward);
    game->gold += enemy->is_boss ? 75 : 10 + reward;
    game->mana = game->max_mana;
    if (!game->quest_hunter_done && game->normal_kills >= 3) {
        game->quest_hunter_done = 1;
        quest_reward = 35;
        gain_xp(game, quest_reward);
    }

    if (enemy->is_boss) {
        snprintf(game->message, sizeof(game->message), "BOSS porazeny! +%d XP, +75 zlata. Vychod X caka!", reward);
    } else if (quest_reward) {
        snprintf(game->message, sizeof(game->message), "Porazil si %s! +%d XP. Uloha 2 splnena: +35 XP!",
                     enemy->name, reward);
    } else {
        snprintf(game->message, sizeof(game->message), "Porazil si %s! +%d XP (%d/3 nepriatelia).",
                     enemy->name, reward, game->normal_kills);
    }
    if (game->player.level > old_level) {
        size_t len = strlen(game->message);
        snprintf(game->message + len, sizeof(game->message) - len,
                 " LEVEL %d!", game->player.level);
    }
}
