#include "game.h"

#include <stdio.h>
#include <string.h>

static const char *endings[DIALOG_ENDINGS] = {
    "Mudrc ti pozehnal. Obnovil ti vsetky HP.",
    "Mudrc ti daroval 25 zlata na cestu.",
    "Mudrc ti prezradil tajomstvo podzemia. +20 XP.",
    "Mudrc ti dal vzacny kralovsky elixir.",
    "Urazil si mudrca. Prisiel si o 10 zlata."
};

/* Koniec dialozu sa zaznamena. Odmena/trest len pri prvom vysledku.
   Vrati 1 pri platnom konci z mnoziny 1..5. */
int dialogue_finish(struct GameState *game, int ending)
{
    int index = ending - 1;
    if (index < 0 || index >= DIALOG_ENDINGS) return 0;
    game->dialogue_last = ending;
    if (game->dialogue_seen[index]) {
        strcpy(game->message, "Tento koniec si uz zazil. Bez dalsej odmeny.");
        return 1;
    }
    game->dialogue_seen[index] = 1;
    if (ending == 1) {
        game->player.hp = game->player.max_hp;
    } else if (ending == 2) {
        game->gold += 25;
    } else if (ending == 3) {
        gain_xp(game, 20);
    } else if (ending == 4) {
        if (game->inventory[13] < 99) game->inventory[13]++;
    } else if (ending == 5) {
        game->gold -= 10;
        if (game->gold < 0) game->gold = 0;
    }
    snprintf(game->message, sizeof(game->message), "Dialog %d/5: %s", ending, endings[index]);
    return 1;
}

static int choose_subdialogue(const char *first, const char *second)
{
    for (;;) {
        char key;
        ui_clear();
        ui_header("ROZHOVOR S MUDRCOM");
        puts("  Mudrc: Co presne potrebujes, cestovatel?");
        printf("\n  [1] %s\n", first);
        printf("  [2] %s\n", second);
        puts("  [Q] Koniec rozhovoru bez rozhodnutia");
        key = read_key();
        if (key == '1' || key == '2') return key - '0';
        if (key == 'q' || key == 0) return 0;
    }
}

void dialogue_screen(struct GameState *game)
{
    for (;;) {
        char key;
        int option;
        int ending = 0;
        ui_clear();
        ui_header("DIALOG - MUDRC V PODZEMI");
        puts("  Mudrc: Vitaj v mojom domove. Co ta trapi?");
        puts("\n  [1] Mozes mi pomoct prezit?");
        puts("  [2] Povedz mi tajomstva podzemia.");
        puts("  [3] Si len stary blazon!");
        puts("  [Q] Rozlucit sa bez odmeny");
        key = read_key();
        if (key == 'q' || key == 0) return;
        if (key == '1') {
            ending = choose_subdialogue("Vyliec moje zranenia.", "Pozicaj mi zlato.");
        } else if (key == '2') {
            option = choose_subdialogue("Nauc ma nieco.", "Daj mi magicky elixir.");
            if (option != 0) ending = option + 2;
        } else if (key == '3') {
            ending = 5;
        }
        if (ending != 0) {
            int already = game->dialogue_seen[ending - 1];
            dialogue_finish(game, ending);
            ui_clear();
            ui_header("KONIEC DIALOGU");
            printf("  KONIEC %d Z 5\n\n", ending);
            if (already)
                puts("  Tento koniec si uz zazil. Odmena sa neopakuje.");
            else
                printf("  %s\n", endings[ending - 1]);
            puts("\n  Mudrc: Dovidenia, cestovatel.");
            ui_wait_key();
            return;
        }
    }
}
