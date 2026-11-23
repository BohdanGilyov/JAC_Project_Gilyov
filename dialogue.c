#include "game.h"

#include <stdio.h>

static const char *endings[DIALOG_ENDINGS] = {
    "Mudrc ti pozehnal. Obnovil ti vsetky HP.",
    "Mudrc ti daroval 25 zlata na cestu.",
    "Mudrc ti prezradil tajomstvo podzemia. +20 XP.",
    "Mudrc ti dal vzacny kralovsky elixir.",
    "Urazil si mudrca. Prisiel si o 10 zlata."
};

/* Koniec dialozu sa zaznamena. Odmena/trest len pri prvom vysledku.
   Vrati 1 pri platnom konci z mnoziny 1..5. */
int dialogue_finish(GameState *game, int ending)
{
    int index = ending - 1;
    if (index < 0 || index >= DIALOG_ENDINGS) return 0;
    game->dialogue_last = ending;
    if (!game->dialogue_seen[index]) {
        game->dialogue_seen[index] = 1;
        switch (ending) {
            case 1:
                game->player.hp = game->player.max_hp;
                break;
            case 2:
                game->gold += 25;
                break;
            case 3:
                gain_xp(game, 20);
                break;
            case 4:
                if (game->inventory[13] < 99) game->inventory[13]++;
                break;
            case 5:
                game->gold = game->gold >= 10 ? game->gold - 10 : 0;
                break;
            default:
                break;
        }
    }
    game_message(game, "Dialog %d/5: %s", ending, endings[index]);
    return 1;
}

static int choose_subdialogue(const char *first, const char *second)
{
    for (;;) {
        char key;
        ui_clear();
        ui_header("ROZHOVOR S MUDRCOM");
        ui_puts("  Mudrc: Co presne potrebujes, cestovatel?");
        ui_printf("\n  [1] %s\n", first);
        ui_printf("  [2] %s\n", second);
        ui_puts("  [Q] Koniec rozhovoru bez rozhodnutia");
        key = read_key();
        if (key == '1' || key == '2') return key - '0';
        if (key == 'q' || key == 0) return 0;
    }
}

void dialogue_screen(GameState *game)
{
    for (;;) {
        char key;
        int option;
        int ending = 0;
        ui_clear();
        ui_header("DIALOG - MUDRC V PODZEMI");
        ui_puts("  Mudrc: Vitaj v mojom domove. Co ta trapi?");
        ui_puts("\n  [1] Mozes mi pomoct prezit?");
        ui_puts("  [2] Povedz mi tajomstva podzemia.");
        ui_puts("  [3] Si len stary blazon!");
        ui_puts("  [Q] Rozlucit sa bez odmeny");
        key = read_key();
        if (key == 'q' || key == 0) return;
        switch (key) {
            case '1':
                option = choose_subdialogue("Vyliec moje zranenia.", "Pozicaj mi zlato.");
                if (option) ending = option; /* 1 alebo 2 */
                break;
            case '2':
                option = choose_subdialogue("Nauc ma nieco.", "Daj mi magicky elixir.");
                if (option) ending = option + 2; /* 3 alebo 4 */
                break;
            case '3':
                ending = 5;
                break;
            default:
                break;
        }
        if (ending != 0) {
            int already = game->dialogue_seen[ending - 1];
            (void)dialogue_finish(game, ending);
            ui_clear();
            ui_header("KONIEC DIALOGU");
            ui_printf("  KONIEC %d Z 5\n\n", ending);
            ui_printf("  %s\n", endings[ending - 1]);
            if (already)
                ui_puts("  Tento koniec si uz zazil. Odmena sa neopakuje.");
            ui_puts("\n  Mudrc: Dovidenia, cestovatel.");
            ui_wait_key();
            return;
        }
    }
}
