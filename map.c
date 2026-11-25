#include "game.h"

#include <stdio.h>

static const char dungeon[MAP_ROWS][MAP_COLS + 1] = {
    "########",
    "#.M.#..#",
    "#.#N...#",
    "#..H#..#",
    "##....##",
    "#..#...#",
    "#.....X#",
    "########"
};

char map_tile(int row, int col)
{
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) {
        return '#';
    }
    return dungeon[row][col];
}

void print_map(const struct GameState *game)
{
    puts("           1  2  3  4  5  6  7  8");
    for (int row = 0; row < MAP_ROWS; row++) {
        printf("        %d  ", row + 1);
        for (int col = 0; col < MAP_COLS; col++) {
            char tile = map_tile(row, col);
            for (int i = 0; i < ENEMY_COUNT; i++) {
                const struct Enemy *e = &game->enemies[i];
                if (e->alive && e->row == row && e->col == col) {
                    tile = e->is_boss ? 'B' : 'E';
                }
            }
            if (row == 3 && col == 3 && game->shrine_used) {
                tile = 'h';
            }
            if (game->player.row == row && game->player.col == col) {
                tile = '@';
            }
            printf("%c  ", tile);
        }
        putchar('\n');
    }
    puts("   @ hrac   # stena   E nepriatel   B boss");
    puts("   H svatyna   h pouzita   X vychod   M obchod   N mudrc");
}

void move_player(struct GameState *game, int dr, int dc)
{
    int row = game->player.row + dr;
    int col = game->player.col + dc;
    char tile = map_tile(row, col);
    int fought = 0;

    if (tile == '#') {
        snprintf(game->message, sizeof(game->message), "Tymto smerom je stena. Skus inu cestu.");
        return;
    }

    if (tile == 'X' && (!game->boss_defeated ||
                        !game->quest_shrine_done || !game->quest_hunter_done)) {
        snprintf(game->message, sizeof(game->message), "Vychod X je zamknuty! Dokonci ulohy a poraz bossa.");
        return;
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        struct Enemy *enemy = &game->enemies[i];
        if (enemy->alive && enemy->row == row && enemy->col == col) {
            if (!start_combat(game, enemy)) {
                return;
            }
            on_enemy_defeated(game, i);
            fought = 1;
            break;
        }
    }

    game->player.row = row;
    game->player.col = col;

    if (fought) {
        return;
    }

    if (tile == 'H' && !game->shrine_used) {
        game->shrine_used = 1;
        game->quest_shrine_done = 1;
        gain_xp(game, 20);
        game->player.hp = game->player.max_hp;
        game->mana = game->max_mana;
        snprintf(game->message, sizeof(game->message), "ULOHA 1 SPLNENA! Svatyna obnovila HP, +20 XP. Level %d.",
                     game->player.level);
    } else if (tile == 'X') {
        game->quest_escape_done = 1;
        snprintf(game->message, sizeof(game->message), "VITAZSTVO! Dokoncil si vsetky 3 ulohy.");
    } else if (tile == 'M') {
        snprintf(game->message, sizeof(game->message), "Si v obchode. Stlac O a obchoduj bez Enter.");
    } else if (tile == 'N') {
        snprintf(game->message, sizeof(game->message), "Stretol si mudrca. Stlac T a rozpravaj sa.");
    } else if (tile == 'H') {
        snprintf(game->message, sizeof(game->message), "Svatyna uz bola pouzita.");
    } else {
        snprintf(game->message, sizeof(game->message), "Pohyb: riadok %d, stlpec %d. Preskumaj podzemie.",
                     row + 1, col + 1);
    }
}
