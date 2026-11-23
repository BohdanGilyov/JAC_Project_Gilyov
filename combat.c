#include "game.h"

#include <stdio.h>

static int damage(int attack, int defense)
{
    int value = attack - defense;
    return value > 0 ? value : 1;
}

int start_combat(GameState *game, Enemy *enemy)
{
    Player *p = &game->player;
    char message[160];
    snprintf(message, sizeof(message), "Stretol si %s! A utok, D obrana, K schopnosti.",
             enemy->name);

    while (p->hp > 0 && enemy->hp > 0) {
        char key;
        int dealt = 0;
        int taken = 0;
        int block_next = 0;
        int action_done = 0;
        char action_message[120] = "";

        ui_battle(game, enemy, message);
        ui_printf("  Mana: %d/%d   [K] Schopnosti   [I] Inventar/elixiry\n",
                  game->mana, game->max_mana);
        key = read_key();
        if (key == 0) {
            game_message(game, "Vstup bol ukonceny; boj sa neskoncil.");
            return 0;
        }

        switch (key) {
            case 'a':
                dealt = damage(effective_attack(game), enemy->defense);
                enemy->hp -= dealt;
                action_done = 1;
                snprintf(action_message, sizeof(action_message),
                         "Zasiahol si za %d HP.", dealt);
                break;
            case 'd':
                action_done = 1;
                snprintf(action_message, sizeof(action_message), "Branis sa.");
                break;
            case 'k': {
                int skill = battle_ability_menu(game);
                if (skill < 0) continue;
                if (!use_ability(game, enemy, skill, &block_next,
                                 action_message, sizeof(action_message))) {
                    snprintf(message, sizeof(message),
                             "Schopnost je zamknuta, nemas manu alebo mas plne HP.");
                    continue;
                }
                action_done = 1;
                break;
            }
            case 'i':
                action_done = inventory_screen(game, 1);
                if (!action_done) continue;
                snprintf(action_message, sizeof(action_message), "%.115s", game->message);
                break;
            default:
                snprintf(message, sizeof(message), "A utok, D obrana, K schopnosti, I elixiry.");
                break;
        }
        if (!action_done) continue;
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

    game_message(game, "PREHRA! %s ta porazil. Mozes nacitat save.", enemy->name);
    return 0;
}
