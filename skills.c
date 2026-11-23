#include "game.h"

#include <stdio.h>

static const char *names[5] = {
    "Silny uder", "Liecivy dotyk", "Stitova stena", "Ohniva gula", "Dvojity zasah"
};

static const char *descriptions[5] = {
    "zvyseny utocny zasah", "obnovi 30 HP", "zablokuje dalsi utok",
    "30 magickych bodov poskodenia", "dva zasahy po sebe"
};

static const int costs[5] = {3, 4, 3, 6, 7};

int ability_unlocked(const GameState *game, int index)
{
    return index >= 0 && index < 5 && game->player.level >= index + 1;
}

int ability_cost(int index)
{
    return index >= 0 && index < 5 ? costs[index] : 0;
}

static int damage(int attack, int defense)
{
    int value = attack - defense;
    return value > 0 ? value : 1;
}

/* Vrati 1, ked sa schopnost pouzila, inak 0 bez spotreby tahu a many. */
int use_ability(GameState *game, Enemy *enemy, int index,
                int *block_next, char *message, size_t capacity)
{
    int dealt;
    if (!enemy || !block_next || !message || capacity == 0 ||
        enemy->hp <= 0 || !ability_unlocked(game, index) ||
        game->mana < ability_cost(index))
        return 0;
    if (index == 1 && game->player.hp >= game->player.max_hp)
        return 0;

    game->mana -= costs[index];
    switch (index) {
        case 0:
            dealt = damage(effective_attack(game) + 10, enemy->defense);
            enemy->hp -= dealt;
            snprintf(message, capacity, "Silny uder: %d poskodenia!", dealt);
            break;
        case 1: {
            int heal = game->player.max_hp - game->player.hp;
            if (heal > 30) heal = 30;
            game->player.hp += heal;
            snprintf(message, capacity, "Liecivy dotyk: +%d HP.", heal);
            break;
        }
        case 2:
            *block_next = 1;
            snprintf(message, capacity, "Stitova stena: zablokoval si nasledujuci utok!");
            break;
        case 3:
            dealt = 30;
            enemy->hp -= dealt;
            snprintf(message, capacity, "Ohniva gula: %d poskodenia bez obrany!", dealt);
            break;
        case 4:
            dealt = 2 * damage(effective_attack(game), enemy->defense);
            enemy->hp -= dealt;
            snprintf(message, capacity, "Dvojity zasah: %d poskodenia!", dealt);
            break;
        default:
            return 0;
    }
    if (enemy->hp < 0) enemy->hp = 0;
    return 1;
}

static void print_abilities(const GameState *game)
{
    for (int i = 0; i < 5; i++) {
        ui_printf("  [%d] %-16s | level %d | %d many | %s\n",
                  i + 1, names[i], i + 1, costs[i],
                  ability_unlocked(game, i) ? "ODOMKNUTE" : "ZAMKNUTE");
        ui_printf("      %s\n", descriptions[i]);
    }
}

int battle_ability_menu(const GameState *game)
{
    ui_clear();
    ui_header("SCHOPNOSTI - VYBER POCAS BOJA");
    ui_printf("  Level: %d   Mana: %d/%d\n\n", game->player.level,
              game->mana, game->max_mana);
    print_abilities(game);
    ui_puts("\n  [1-5] Pouzit schopnost     [Q] Vratit sa do boja");
    char key = read_key();
    if (key >= '1' && key <= '5') return key - '1';
    return -1;
}

void skills_screen(const GameState *game)
{
    ui_clear();
    ui_header("SCHOPNOSTI - 5 SCHOPNOSTI");
    ui_printf("  Level: %d   Mana: %d/%d\n\n", game->player.level,
              game->mana, game->max_mana);
    print_abilities(game);
    ui_puts("\n  Schopnosti sa odomykaju kazdym novym levelom.");
    ui_puts("  Pocas boja stlac K, potom cislicu 1-5.");
    ui_puts("  Mana sa obnovuje po boji a pri navsteve svatyne.");
    ui_wait_key();
}
