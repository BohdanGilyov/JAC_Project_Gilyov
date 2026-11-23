#include "game.h"

#include <ctype.h>
#include <stdio.h>

static const int level_xp[MAX_LEVEL] = {0, 20, 50, 90, 140};

void init_game(GameState *game)
{
    *game = (GameState){0};

    game->player = (Player){1, 1, 100, 100, 12, 3, 1, 0};

    game->enemies[0] = (Enemy){"Rat",       2, 6, 15,  5, 1, 15, 1, 0};
    game->enemies[1] = (Enemy){"Skeleton",  5, 1, 25,  8, 2, 20, 1, 0};
    game->enemies[2] = (Enemy){"Goblin",    4, 4, 35,  9, 3, 25, 1, 0};
    game->enemies[3] = (Enemy){"Orc",       5, 6, 45, 11, 4, 30, 1, 0};
    game->enemies[4] = (Enemy){"Boss",      6, 5, 70, 14, 5, 60, 1, 1};
}

void show_player(const GameState *game)
{
    const Player *p = &game->player;
    printf("\n--- HRAC ---\n");
    printf("Level: %d/%d | XP: %d", p->level, MAX_LEVEL, p->xp);
    if (p->level < MAX_LEVEL) {
        printf(" (dalsi level: %d XP)", level_xp[p->level]);
    }
    printf("\nHP: %d/%d | Utok: %d | Obrana: %d\n",
           p->hp, p->max_hp, p->attack, p->defense);
    printf("Porazeni nepriatelia: %d/4\n", game->normal_kills);
}

void show_quests(const GameState *game)
{
    printf("\n--- ULOHY ---\n");
    printf("[%c] 1. Najdi liecivu svatynu (20 XP)\n",
           game->quest_shrine_done ? 'X' : ' ');
    printf("[%c] 2. Poraz 3 beznych nepriatelov (%d/3, 35 XP)\n",
           game->quest_hunter_done ? 'X' : ' ',
           game->normal_kills < 3 ? game->normal_kills : 3);
    printf("[%c] 3. Poraz bossa a prejdi vychodom\n",
           game->quest_escape_done ? 'X' : ' ');
}

void gain_xp(GameState *game, int amount)
{
    Player *p = &game->player;
    p->xp += amount;
    printf("Ziskavas %d XP. Celkovo: %d XP.\n", amount, p->xp);

    while (p->level < MAX_LEVEL && p->xp >= level_xp[p->level]) {
        p->level++;
        p->max_hp += 10;
        p->attack += 2;
        p->defense++;
        p->hp = p->max_hp;
        printf("LEVEL UP! Teraz mas level %d. HP sa obnovilo.\n", p->level);
    }
}

void on_enemy_defeated(GameState *game, int enemy_index)
{
    Enemy *enemy = &game->enemies[enemy_index];
    enemy->alive = 0;
    enemy->hp = 0;

    if (enemy->is_boss) {
        game->boss_defeated = 1;
        printf("Boss je porazeny! Vychod X je odomknuty, ak mas aj ostatne ulohy.\n");
    } else {
        game->normal_kills++;
    }

    gain_xp(game, enemy->xp_reward);

    if (!game->quest_hunter_done && game->normal_kills >= 3) {
        game->quest_hunter_done = 1;
        printf("ULOHA SPLNENA: Poraz 3 beznych nepriatelov!\n");
        gain_xp(game, 35);
    }
}

char read_command(void)
{
    char input[64];
    int c;
    size_t i = 0;

    if (!fgets(input, sizeof(input), stdin)) {
        return '\0';
    }

    while (input[i] && isspace((unsigned char)input[i])) {
        i++;
    }
    if (!input[i]) {
        return '\n';
    }

    c = (unsigned char)input[i];
    i++;
    while (input[i] && isspace((unsigned char)input[i])) {
        i++;
    }
    if (input[i]) {
        return '?';
    }

    return (char)tolower(c);
}
