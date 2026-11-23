#include "game.h"

#include <stdio.h>

static void show_help(void)
{
    printf("\n--- OVLADANIE ---\n");
    printf("W/A/S/D - pohyb (potom Enter)\n");
    printf("P - statistiky  U - ulohy  V - ulozit hru\n");
    printf("H - pomoc       Q - navrat do hlavneho menu\n");
    printf("V boji: A - utok, D - obrana (potom Enter)\n");
}

static void play(GameState *game)
{
    char command;

    show_help();
    while (game->player.hp > 0 && !game->quest_escape_done) {
        print_map(game);
        printf("\nHP %d/%d | Level %d | XP %d\n",
               game->player.hp, game->player.max_hp,
               game->player.level, game->player.xp);
        printf("Pohyb W/A/S/D | P=hrac U=ulohy V=save H=pomoc Q=menu\n> ");
        command = read_command();

        switch (command) {
            case 'w': move_player(game, -1, 0); break;
            case 's': move_player(game, 1, 0); break;
            case 'a': move_player(game, 0, -1); break;
            case 'd': move_player(game, 0, 1); break;
            case 'p': show_player(game); break;
            case 'u': show_quests(game); break;
            case 'v':
                if (save_game(game, SAVE_FILE)) {
                    printf("Hra bola ulozena do %s.\n", SAVE_FILE);
                } else {
                    printf("Nepodarilo sa ulozit hru.\n");
                }
                break;
            case 'h': show_help(); break;
            case 'q':
                printf("Navrat do hlavneho menu. Neulozeny postup sa strati.\n");
                return;
            case '\0':
                printf("Koniec vstupu.\n");
                return;
            default:
                printf("Neznamy prikaz. Stlac H pre pomoc.\n");
                break;
        }
    }

    if (game->quest_escape_done) {
        show_player(game);
        show_quests(game);
    }
}

int main(void)
{
    GameState game;
    char command;

    printf("======================================\n");
    printf("        DUNGEON ESCAPE (C)\n");
    printf("======================================\n");

    for (;;) {
        printf("\n--- HLAVNE MENU ---\n");
        printf("[N] Nova hra\n[L] Nacitat hru\n[Q] Koniec\n> ");
        command = read_command();

        switch (command) {
            case 'n':
                init_game(&game);
                play(&game);
                break;
            case 'l':
                if (load_game(&game, SAVE_FILE)) {
                    printf("Hra bola nacitana z %s.\n", SAVE_FILE);
                    play(&game);
                } else {
                    printf("Save neexistuje alebo je poskodeny (%s).\n", SAVE_FILE);
                }
                break;
            case 'q':
            case '\0':
                printf("Dakujeme za hranie!\n");
                return 0;
            default:
                printf("Neplatna volba. Pouzi N, L alebo Q.\n");
                break;
        }
    }
}
