#include "game.h"
#include <stdio.h>

static void show_extended_help(void)
{
    ui_clear();
    ui_header("POMOC - OVLADANIE BEZ ENTER");
    ui_puts("  MAPA: W/A/S/D pohyb | P statistiky | U ulohy | V ulozit");
    ui_puts("  I inventar | K schopnosti | H pomoc | Q hlavne menu");
    ui_puts("  O obchod na policku M | T rozhovor na policku N");
    ui_puts("  INVENTAR: W zbrane, A brnenie, P elixiry, 1-5 vyber, Q spat");
    ui_puts("  OBCHOD: W/A/P kategoria, B kupit, S predat, 1-5 predmet");
    ui_puts("  BOJ: A utok, D obrana, K schopnosti, I elixiry/vybava");
    ui_puts("  SCHOPNOSTI: odomykaju sa od levelu 1 po 5; stoja manu.");
    ui_puts("  DIALOG: zvol 1/2/3 a dalsiu odpoved; 5 koncov rozhovoru.");
    ui_puts("  CIEL: svatyna H, 3 nepriatelia, boss B, potom vychod X.");
    ui_puts("  Vsetko funguje jednym stlacenim klavesu (bez Enter).");
    ui_wait_key();
}

static void play(GameState *game)
{
    while (game->player.hp > 0 && !game->quest_escape_done) {
        char key;
        ui_world(game);
        ui_printf("  ZLATO %d  |  MANA %d/%d  |  UTOCNA SILA %d  |  OBRANA %d\n",
                  game->gold, game->mana, game->max_mana,
                  effective_attack(game), effective_defense(game));
        ui_puts("  [I] inventar  [K] schopnosti  [O] obchod na M  [T] dialog na N");
        key = read_key();

        switch (key) {
            case 'w': move_player(game, -1, 0); break;
            case 's': move_player(game, 1, 0); break;
            case 'a': move_player(game, 0, -1); break;
            case 'd': move_player(game, 0, 1); break;
            case 'p': ui_stats(game); break;
            case 'i': (void)inventory_screen(game, 0); break;
            case 'k': skills_screen(game); break;
            case 'o':
                if (map_tile(game->player.row, game->player.col) == 'M')
                    shop_screen(game);
                else
                    game_message(game, "Na nakup musis stat na M (obchodnik).");
                break;
            case 't':
                if (map_tile(game->player.row, game->player.col) == 'N')
                    dialogue_screen(game);
                else
                    game_message(game, "Na rozhovor musis stat na N (mudrc).");
                break;
            case 'u': ui_quests(game); break;
            case 'h': show_extended_help(); break;
            case 'v':
                if (save_game(game, SAVE_FILE)) {
                    game_message(game, "Hra ulozena do %s.", SAVE_FILE);
                } else {
                    game_message(game, "CHYBA: nepodarilo sa ulozit hru.");
                }
                break;
            case 'q':
                ui_clear();
                ui_header("NAVRAT DO MENU");
                ui_puts("  Neulozeny postup sa strati.");
                ui_puts("  [Y] Ano, do menu    [N] Nie, pokracovat");
                if (read_key() == 'y') {
                    return;
                }
                game_message(game, "Pokracuj v prieskume.");
                break;
            case 0:
                return;
            default:
                game_message(game, "Neplatna volba. Stlac H pre pomoc (bez Enter).");
                break;
        }
    }

    ui_end(game, game->quest_escape_done);
}

int main(void)
{
    GameState game;
    const char *menu_message = "Vyber jednu moznost jednym stlacenim klavesu.";

    ui_init();
    while (1) {
        char key;
        ui_main_menu(menu_message);
        key = read_key();

        switch (key) {
            case 'n':
                init_game(&game);
                play(&game);
                menu_message = "Vitaj spat v hlavnom menu.";
                break;
            case 'l':
                if (load_game(&game, SAVE_FILE)) {
                    game_message(&game, "Ulozena hra bola uspesne nacitana.");
                    play(&game);
                    menu_message = "Vitaj spat v hlavnom menu.";
                } else {
                    menu_message = "Save neexistuje alebo je poskodeny.";
                }
                break;
            case 'h':
                show_extended_help();
                menu_message = "Vyber novu hru alebo nacitanie.";
                break;
            case 'q':
            case 0:
                ui_clear();
                ui_header("KONIEC");
                ui_puts("  Dakujeme za hranie!");
                ui_present();
                return 0;
            default:
                menu_message = "Neplatna volba! Stlac N, L, H alebo Q.";
                break;
        }
    }
}
