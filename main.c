#include "game.h"
#include <stdio.h>
#include <string.h>

static void show_extended_help(void)
{
    ui_clear();
    ui_header("POMOC - OVLADANIE BEZ ENTER");
    puts("  MAPA: W/A/S/D pohyb | P statistiky | U ulohy | V ulozit");
    puts("  I inventar | K schopnosti | H pomoc | Q hlavne menu");
    puts("  O obchod na policku M | T rozhovor na policku N");
    puts("  INVENTAR: W zbrane, A brnenie, P elixiry, 1-5 vyber, Q spat");
    puts("  OBCHOD: W/A/P kategoria, B kupit, S predat, 1-5 predmet");
    puts("  BOJ: A utok, D obrana, K schopnosti, I elixiry/vybava");
    puts("  SCHOPNOSTI: odomykaju sa od levelu 1 po 5; stoja manu.");
    puts("  DIALOG: zvol 1/2/3 a dalsiu odpoved; 5 koncov rozhovoru.");
    puts("  CIEL: svatyna H, 3 nepriatelia, boss B, potom vychod X.");
    puts("  Vsetko funguje jednym stlacenim klavesu (bez Enter).");
    ui_wait_key();
}

static void play(struct GameState *game)
{
    while (game->player.hp > 0 && !game->quest_escape_done) {
        char key;
        ui_world(game);
        key = read_key();

        if (key == 'w') move_player(game, -1, 0);
        else if (key == 's') move_player(game, 1, 0);
        else if (key == 'a') move_player(game, 0, -1);
        else if (key == 'd') move_player(game, 0, 1);
        else if (key == 'p') ui_stats(game);
        else if (key == 'i') inventory_screen(game, 0);
        else if (key == 'k') skills_screen(game);
        else if (key == 'u') ui_quests(game);
        else if (key == 'h') show_extended_help();
        else if (key == 'o') {
            if (map_tile(game->player.row, game->player.col) == 'M')
                shop_screen(game);
            else
                strcpy(game->message, "Na nakup musis stat na M.");
        } else if (key == 't') {
            if (map_tile(game->player.row, game->player.col) == 'N')
                dialogue_screen(game);
            else
                strcpy(game->message, "Na rozhovor musis stat na N.");
        } else if (key == 'v') {
            if (save_game(game, SAVE_FILE))
                strcpy(game->message, "Hra ulozena do save.txt.");
            else
                strcpy(game->message, "CHYBA: nepodarilo sa ulozit hru.");
        } else if (key == 'q') {
            ui_clear();
            ui_header("NAVRAT DO MENU");
            puts("Neulozeny postup sa strati.");
            puts("[Y] Ano, do menu | [N] Nie, pokracovat");
            if (read_key() == 'y') return;
        } else if (key == 0) {
            return;
        } else {
            strcpy(game->message, "Neplatna volba. Stlac H pre pomoc.");
        }
    }

    ui_end(game, game->quest_escape_done);
}

int main(void)
{
    struct GameState game;
    const char *menu_message = "Vyber jednu moznost jednym stlacenim klavesu.";

    terminal_prepare();
    while (1) {
        char key;
        ui_main_menu(menu_message);
        key = read_key();

        if (key == 'n') {
            init_game(&game);
            play(&game);
            menu_message = "Vitaj spat v hlavnom menu.";
        } else if (key == 'l') {
            if (load_game(&game, SAVE_FILE)) {
                strcpy(game.message, "Ulozena hra bola uspesne nacitana.");
                play(&game);
                menu_message = "Vitaj spat v hlavnom menu.";
            } else {
                menu_message = "Save neexistuje alebo je poskodeny.";
            }
        } else if (key == 'h') {
            show_extended_help();
        } else if (key == 'q' || key == 0) {
            ui_clear();
            puts("Dakujeme za hranie!");
            return 0;
        } else {
            menu_message = "Neplatna volba! Stlac N, L, H alebo Q.";
        }
    }
}
