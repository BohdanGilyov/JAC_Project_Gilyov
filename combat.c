#include "game.h"

#include <stdio.h>
#include <string.h>

static int damage(int attack, int defense)
{
    int value = attack - defense;
    return value > 0 ? value : 1;
}

int start_combat(struct GameState *game, struct Enemy *enemy)
{
    struct Player *p = &game->player;
    char message[160];
    snprintf(message, sizeof(message), "Stretol si %s! A utok, D obrana, K schopnosti.",
             enemy->name);

    while (p->hp > 0 && enemy->hp > 0) {
        char key;
        int dealt = 0;
        int taken = 0;
        int block_next = 0;
        char action_message[120] = "";

        ui_battle(game, enemy, message);
        printf("  Mana: %d/%d   [K] Schopnosti   [I] Inventar/elixiry\n",
                  game->mana, game->max_mana);
        key = read_key();
        if (key == 0) {
            snprintf(game->message, sizeof(game->message), "Vstup bol ukonceny; boj sa neskoncil.");
            return 0;
        }

        if (key == 'a') {
            dealt = damage(effective_attack(game), enemy->defense);
            enemy->hp -= dealt;
            snprintf(action_message, sizeof(action_message), "Zasiahol si za %d HP.", dealt);
        } else if (key == 'd') {
            strcpy(action_message, "Branis sa.");
        } else if (key == 'k') {
            int skill = battle_ability_menu(game);
            if (skill < 0) continue;
            if (!use_ability(game, enemy, skill, &block_next,
                             action_message, sizeof(action_message))) {
                strcpy(message, "Schopnost je zamknuta, nemas manu alebo mas plne HP.");
                continue;
            }
        } else if (key == 'i') {
            if (!inventory_screen(game, 1)) continue;
            snprintf(action_message, sizeof(action_message), "%.115s", game->message);
        } else {
            strcpy(message, "A utok, D obrana, K schopnosti, I elixiry.");
            continue;
        }
        if (enemy->hp <= 0) {
            enemy->hp = 0;
            return 1;
        }
        if (block_next) {
            taken = 0;
        } else {
            taken = damage(enemy->attack, effective_defense(game));
            if (key == 'd') taken = (taken + 1) / 2;
        }
        p->hp -= taken;
        if (p->hp < 0) p->hp = 0;
        snprintf(message, sizeof(message), "%s %s udelil %d HP.",
                 action_message, enemy->name, taken);
    }

    snprintf(game->message, sizeof(game->message), "PREHRA! %s ta porazil. Mozes nacitat save.", enemy->name);
    return 0;
}
