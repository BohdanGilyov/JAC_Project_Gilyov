#include "game.h"

#include <stdio.h>

static const char dungeon[MAP_ROWS][MAP_COLS + 1] = {
    "########",
    "#...#..#",
    "#.#....#",
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
    printf("\n--- PODZEMIE ---\n");
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            char tile = map_tile(row, col);

            for (int i = 0; i < ENEMY_COUNT; i++) {
                const Enemy *enemy = &game->enemies[i];
                if (enemy->alive && enemy->row == row && enemy->col == col) {
                    tile = enemy->is_boss ? 'B' : 'E';
                }
            }
            if (row == 3 && col == 3 && game->shrine_used) {
                tile = 'h';
            }
            if (game->player.row == row && game->player.col == col) {
                tile = '@';
            }
            printf("%c ", tile);
        }
        putchar('\n');
    }
    printf("@=hrac  #=stena  E=nepriatel  B=boss  H=svatyna  X=vychod\n");
}

void move_player(GameState *game, int dr, int dc)
{
    const int row = game->player.row + dr;
    const int col = game->player.col + dc;
    const char tile = map_tile(row, col);

    if (tile == '#') {
        printf("Tymto smerom je stena.\n");
        return;
    }

    if (tile == 'X' && (!game->boss_defeated ||
                        !game->quest_shrine_done || !game->quest_hunter_done)) {
        printf("Vychod je zamknuty. Najprv spln prve dve ulohy a poraz bossa.\n");
        return;
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        Enemy *enemy = &game->enemies[i];
        if (enemy->alive && enemy->row == row && enemy->col == col) {
            if (!start_combat(game, enemy)) {
                return;
            }
            on_enemy_defeated(game, i);
            break;
        }
    }

    game->player.row = row;
    game->player.col = col;

    if (tile == 'H' && !game->shrine_used) {
        game->shrine_used = 1;
        game->player.hp = game->player.max_hp;
        game->quest_shrine_done = 1;
        printf("Nasiel si liecivu svatynu! Zdravie je obnovene.\n");
        printf("ULOHA SPLNENA: Najdi liecivu svatynu!\n");
        gain_xp(game, 20);
    }

    if (tile == 'X') {
        game->quest_escape_done = 1;
        printf("\n======================================\n");
        printf(" VITAZSTVO! USPESNE SI UNIKOL!\n");
        printf(" Vsetky tri ulohy su splnene.\n");
        printf("======================================\n");
    }
}
