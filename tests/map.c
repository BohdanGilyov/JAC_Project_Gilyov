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

void print_map(const GameState *game)
{
    ui_puts("           1  2  3  4  5  6  7  8");
    for (int row = 0; row < MAP_ROWS; row++) {
        ui_printf("        %d  ", row + 1);
        for (int col = 0; col < MAP_COLS; col++) {
            char tile = map_tile(row, col);
            for (int i = 0; i < ENEMY_COUNT; i++) {
                const Enemy *e = &game->enemies[i];
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
            ui_printf("%c  ", tile);
        }
        ui_putchar('\n');
    }
    ui_puts("   @ hrac   # stena   E nepriatel   B boss");
    ui_puts("   H svatyna   h pouzita   X vychod   M obchod   N mudrc");
}

void move_player(GameState *game, int dr, int dc)
{
    int row = game->player.row + dr;
    int col = game->player.col + dc;
    char tile = map_tile(row, col);
    int fought = 0;

    if (tile == '#') {
        game_message(game, "Tymto smerom je stena. Skus inu cestu.");
        return;
    }

    if (tile == 'X' && (!game->boss_defeated ||
                        !game->quest_shrine_done || !game->quest_hunter_done)) {
        game_message(game, "Vychod X je zamknuty! Dokonci ulohy a poraz bossa.");
        return;
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        Enemy *enemy = &game->enemies[i];
        if (enemy->alive && enemy->row == row && enemy->col == col) {
            if (!start_combat(game, enemy)) {
                return;
            }
            /* Od porazenia po vypis odmeny sa HP nesmie doplnit. */
            const int hp_after_combat = game->player.hp;
            on_enemy_defeated(game, i);
            game->player.hp = hp_after_combat;
            fought = 1;
            break;
        }
    }

    game->player.row = row;
    game->player.col = col;

    if (tile == 'H' && !game->shrine_used) {
        game->shrine_used = 1;
        game->quest_shrine_done = 1;
        gain_xp(game, 20);
        game->player.hp = game->player.max_hp;
        game->mana = game->max_mana;
        game_message(game, "ULOHA 1 SPLNENA! Svatyna obnovila HP, +20 XP. Level %d.",
                     game->player.level);
    } else if (tile == 'X') {
        game->quest_escape_done = 1;
        game_message(game, "VITAZSTVO! Dokoncil si vsetky 3 ulohy.");
    } else if (tile == 'M') {
        game_message(game, "Si v obchode. Stlac O a obchoduj bez Enter.");
    } else if (tile == 'N') {
        game_message(game, "Stretol si mudrca. Stlac T a rozpravaj sa.");
    } else if (tile == 'H') {
        game_message(game, "Svatyna uz bola pouzita.");
    } else if (!fought) {
        game_message(game, "Pohyb: riadok %d, stlpec %d. Preskumaj podzemie.",
                     row + 1, col + 1);
    }
}
