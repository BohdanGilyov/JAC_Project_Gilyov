#include "game.h"

#include <stdarg.h>
#include <stdio.h>

static const int level_xp[MAX_LEVEL] = {0, 20, 50, 90, 140};

void game_message(GameState *game, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(game->message, sizeof(game->message), format, args);
    va_end(args);
}

void init_game(GameState *game)
{
    *game = (GameState){0};
    game->player = (Player){1, 1, 100, 100, 12, 3, 1, 0};

    game->enemies[0] = (Enemy){"Rat",      2, 6, 15, 15,  5, 1, 15, 1, 0};
    game->enemies[1] = (Enemy){"Skeleton", 5, 1, 25, 25,  8, 2, 20, 1, 0};
    game->enemies[2] = (Enemy){"Goblin",   4, 4, 35, 35,  9, 3, 25, 1, 0};
    game->enemies[3] = (Enemy){"Orc",      5, 6, 45, 45, 11, 4, 30, 1, 0};
    game->enemies[4] = (Enemy){"Boss",     6, 5, 70, 70, 14, 5, 60, 1, 1};
    game->gold = 70;
    game->mana = 20;
    game->max_mana = 20;
    game->equipped_weapon = -1;
    game->equipped_armor = -1;
    game->inventory[10] = 2; /* 2 male elixiry na zaciatok */
    game_message(game, "Najdi svatynu H, obchod M, mudrca N a unikni cez X!");
}

void gain_xp(GameState *game, int amount)
{
    Player *p = &game->player;
    int old_level = p->level;
    const int hp_before_xp = p->hp;
    p->xp += amount;

    while (p->level < MAX_LEVEL && p->xp >= level_xp[p->level]) {
        p->level++;
        p->max_hp += 10;
        p->attack += 2;
        p->defense++;
        game->max_mana += 5;
        game->mana = game->max_mana;
    }

    /* Level-up zvysuje MAX HP, nikdy nelieci aktualne HP. */
    p->hp = hp_before_xp;

    if (p->level > old_level) {
        game_message(game, "+%d XP! LEVEL UP: %d -> %d. Max HP zvysene!",
                     amount, old_level, p->level);
    } else {
        game_message(game, "Ziskal si %d XP (celkovo %d).", amount, p->xp);
    }
}

void on_enemy_defeated(GameState *game, int enemy_index)
{
    Enemy *enemy;
    int old_level;
    int reward;
    int quest_reward = 0;
    int hp_after_combat;

    if (enemy_index < 0 || enemy_index >= ENEMY_COUNT) {
        return;
    }
    enemy = &game->enemies[enemy_index];
    if (!enemy->alive) {
        return;
    }

    /* Zapamatame skutocne zostavajuce HP po boji.
       Odmeny a ziskane levely nemaju pravo obnovovat zdravie. */
    hp_after_combat = game->player.hp;
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
        game_message(game, "BOSS porazeny! +%d XP, +75 zlata. Vychod X caka!", reward);
    } else if (quest_reward) {
        game_message(game, "Porazil si %s! +%d XP. Uloha 2 splnena: +35 XP!",
                     enemy->name, reward);
    } else {
        game_message(game, "Porazil si %s! +%d XP (%d/3 nepriatelia).",
                     enemy->name, reward, game->normal_kills);
    }
    /* Vsetky odmeny a level-up su ukoncene. HP zostava nezmenene. */
    game->player.hp = hp_after_combat;

    if (game->player.level > old_level) {
        size_t len = 0;
        while (game->message[len] != '\0') {
            len++;
        }
        snprintf(game->message + len, sizeof(game->message) - len,
                 " LEVEL %d!", game->player.level);
    }
    /* Odlišny text oproti starsim buildom; hrac vidi skutocne HP po boji. */
    size_t len = 0;
    while (game->message[len] != '\0') len++;
    snprintf(game->message + len, sizeof(game->message) - len,
             " HP po boji %d/%d (bez liecenia).",
             game->player.hp, game->player.max_hp);
}
