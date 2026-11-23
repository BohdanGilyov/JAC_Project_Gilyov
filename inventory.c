#include "game.h"

#include <stdio.h>

const Item item_catalog[ITEM_COUNT] = {
    {"Hrdzavy mec",     WEAPON,  20,  2},
    {"Kratky mec",      WEAPON,  40,  4},
    {"Ocelovy mec",     WEAPON,  75,  6},
    {"Elfsky mec",      WEAPON, 110,  9},
    {"Mec hrdinu",      WEAPON, 160, 12},
    {"Kozena vesta",    ARMOR,   20,  1},
    {"Retazova kosela", ARMOR,   40,  2},
    {"Rytier brnenie",  ARMOR,   70,  4},
    {"Ocelovy pancier", ARMOR,  105,  6},
    {"Dracia zbroj",    ARMOR,  155,  9},
    {"Maly elixir",     POTION,  12, 15},
    {"Stredny elixir",  POTION,  22, 30},
    {"Velky elixir",    POTION,  35, 50},
    {"Kralovsky elixir",POTION,  54, 80},
    {"Elixir zivota",   POTION,  75, 999}
};

int effective_attack(const GameState *game)
{
    int value = game->player.attack;
    if (game->equipped_weapon >= 0 && game->equipped_weapon < ITEM_PER_TYPE)
        value += item_catalog[game->equipped_weapon].bonus;
    return value;
}

int effective_defense(const GameState *game)
{
    int value = game->player.defense;
    if (game->equipped_armor >= 5 && game->equipped_armor < 10)
        value += item_catalog[game->equipped_armor].bonus;
    return value;
}

int item_buy(GameState *game, int index)
{
    if (index < 0 || index >= ITEM_COUNT ||
        game->inventory[index] >= 99 || game->gold < item_catalog[index].price)
        return 0;
    game->gold -= item_catalog[index].price;
    game->inventory[index]++;
    game_message(game, "Kupene: %s. Zostava %d zlata.",
                 item_catalog[index].name, game->gold);
    return 1;
}

int item_sell(GameState *game, int index)
{
    if (index < 0 || index >= ITEM_COUNT || game->inventory[index] <= 0)
        return 0;
    game->inventory[index]--;
    game->gold += item_catalog[index].price / 2;
    if (game->equipped_weapon == index && game->inventory[index] == 0)
        game->equipped_weapon = -1;
    if (game->equipped_armor == index && game->inventory[index] == 0)
        game->equipped_armor = -1;
    game_message(game, "Predane: %s. Mas %d zlata.",
                 item_catalog[index].name, game->gold);
    return 1;
}

/* Vracia 1 iba po skutocnom pouziti elixiru. */
int item_equip_or_use(GameState *game, int index)
{
    int healed;
    if (index < 0 || index >= ITEM_COUNT || game->inventory[index] <= 0)
        return 0;

    if (item_catalog[index].type == WEAPON) {
        game->equipped_weapon = index;
        game_message(game, "Nasadil si %s (+%d utok).",
                     item_catalog[index].name, item_catalog[index].bonus);
        return 0;
    }
    if (item_catalog[index].type == ARMOR) {
        game->equipped_armor = index;
        game_message(game, "Nasadil si %s (+%d obrana).",
                     item_catalog[index].name, item_catalog[index].bonus);
        return 0;
    }
    if (game->player.hp >= game->player.max_hp)
        return 0;
    healed = item_catalog[index].bonus;
    if (healed > game->player.max_hp - game->player.hp)
        healed = game->player.max_hp - game->player.hp;
    game->player.hp += healed;
    game->inventory[index]--;
    game_message(game, "Pouzil si %s. +%d HP.", item_catalog[index].name, healed);
    return 1;
}

static void print_items(const GameState *game, int category, int in_shop, int mode)
{
    int first = category * ITEM_PER_TYPE;
    for (int i = 0; i < ITEM_PER_TYPE; i++) {
        int index = first + i;
        const Item *item = &item_catalog[index];
        char mark = game->equipped_weapon == index || game->equipped_armor == index ? '*' : ' ';
        ui_printf("  [%d] %-18s %+3d %-6s  mas:%-2d %c",
                  i + 1, item->name, item->bonus,
                  item->type == WEAPON ? "UTOK" : item->type == ARMOR ? "OBRANA" : "HP",
                  game->inventory[index], mark);
        if (in_shop)
            ui_printf("  %d zlata", mode ? item->price / 2 : item->price);
        ui_putchar('\n');
    }
}

int inventory_screen(GameState *game, int in_battle)
{
    int category = POTION;
    char status[100] = "Vyber predmet cislicou 1-5.";
    for (;;) {
        char key;
        int index;
        ui_clear();
        ui_header(in_battle ? "INVENTAR - BOJ" : "INVENTAR");
        ui_printf("  Zlato: %d  |  HP: %d/%d  |  Utok: %d  |  Obrana: %d\n\n",
                  game->gold, game->player.hp, game->player.max_hp,
                  effective_attack(game), effective_defense(game));
        ui_puts("  [W] Zbrane  [A] Brnenie  [P] Elixiry  [Q] Spat");
        ui_printf("\n  Kategoria: %s\n", category == WEAPON ? "ZBRANE" :
                  category == ARMOR ? "BRNENIE" : "ELIXIRY");
        print_items(game, category, 0, 0);
        ui_puts("\n  Cislica 1-5: nasadit predmet alebo vypit elixir.");
        ui_puts("  * oznacuje nasadenu vybavu.");
        ui_printf("\n  INFO: %s\n", status);
        key = read_key();
        if (key == 'q' || key == 0) return 0;
        if (key == 'w') { category = WEAPON; continue; }
        if (key == 'a') { category = ARMOR; continue; }
        if (key == 'p') { category = POTION; continue; }
        if (key < '1' || key > '5') {
            snprintf(status, sizeof(status), "Neplatna moznost.");
            continue;
        }
        index = category * ITEM_PER_TYPE + key - '1';
        if (game->inventory[index] == 0) {
            snprintf(status, sizeof(status), "Tento predmet nemas.");
            continue;
        }
        if (category == POTION && game->player.hp == game->player.max_hp) {
            snprintf(status, sizeof(status), "Mas plne zdravie; elixir sa nespotreboval.");
            continue;
        }
        if (item_equip_or_use(game, index)) return in_battle ? 1 : 0;
        snprintf(status, sizeof(status), "%.90s", game->message);
    }
}

void shop_screen(GameState *game)
{
    int category = WEAPON;
    int sell = 0;
    char status[100] = "Vitaj! Vyber kategoriu a predmet.";
    for (;;) {
        char key;
        int index;
        int success;
        ui_clear();
        ui_header("OBCHODNIK - NAKUP A PREDAJ");
        ui_printf("  Tvoje zlato: %d\n\n", game->gold);
        ui_puts("  [W] Zbrane   [A] Brnenie   [P] Elixiry");
        ui_puts("  [B] Nakup    [S] Predaj    [Q] Odist");
        ui_printf("\n  REZIM: %s   KATEGORIA: %s\n\n", sell ? "PREDAJ" : "NAKUP",
                  category == WEAPON ? "ZBRANE" : category == ARMOR ? "BRNENIE" : "ELIXIRY");
        print_items(game, category, 1, sell);
        ui_puts("\n  Cislicou 1-5 kupis alebo predas vybrany predmet.");
        ui_printf("\n  INFO: %s\n", status);
        key = read_key();
        if (key == 'q' || key == 0) return;
        if (key == 'w') { category = WEAPON; continue; }
        if (key == 'a') { category = ARMOR; continue; }
        if (key == 'p') { category = POTION; continue; }
        if (key == 'b') { sell = 0; continue; }
        if (key == 's') { sell = 1; continue; }
        if (key < '1' || key > '5') {
            snprintf(status, sizeof(status), "Neplatna moznost.");
            continue;
        }
        index = category * ITEM_PER_TYPE + key - '1';
        success = sell ? item_sell(game, index) : item_buy(game, index);
        if (success)
            snprintf(status, sizeof(status), "%.90s", game->message);
        else
            snprintf(status, sizeof(status), sell ?
                     "Predmet nemas; nie je co predat." : "Nedostatok zlata alebo plny inventar.");
    }
}
