#include "game.h"

#include <stdio.h>

static int damage(int attack, int defense)
{
    int result = attack - defense;
    return result > 0 ? result : 1;
}

int start_combat(GameState *game, Enemy *enemy)
{
    Player *p = &game->player;
    printf("\nZACINA BOJ: %s!\n", enemy->name);

    while (p->hp > 0 && enemy->hp > 0) {
        char command;
        int dealt;
        int taken;

        printf("\nTvoje HP: %d/%d | %s HP: %d\n",
               p->hp, p->max_hp, enemy->name, enemy->hp);
        printf("[A] Utok  [D] Obrana (polovicny prijaty utok)\n> ");
        command = read_command();

        if (command == '\0') {
            printf("Vstup bol ukonceny.\n");
            return 0;
        }
        if (command != 'a' && command != 'd') {
            printf("Neplatny prikaz. Pouzi A alebo D.\n");
            continue;
        }

        if (command == 'a') {
            dealt = damage(p->attack, enemy->defense);
            enemy->hp -= dealt;
            if (enemy->hp < 0) {
                enemy->hp = 0;
            }
            printf("Zasiahol si %s za %d HP.\n", enemy->name, dealt);
        } else {
            printf("Branis sa a znizujes dalsi utok na polovicu.\n");
        }

        if (enemy->hp == 0) {
            printf("Vyhral si boj proti %s!\n", enemy->name);
            return 1;
        }

        taken = damage(enemy->attack, p->defense);
        if (command == 'd') {
            taken = (taken + 1) / 2;
        }
        p->hp -= taken;
        if (p->hp < 0) {
            p->hp = 0;
        }
        printf("%s ti sposobil %d poskodenia.\n", enemy->name, taken);
    }

    if (p->hp == 0) {
        printf("\nPREHRAL SI. Hra sa skoncila. Mozes nacitat starsi save.\n");
    }
    return 0;
}
