#ifndef GAME_H
#define GAME_H

#include <stddef.h>

#define MAP_ROWS 8
#define MAP_COLS 8
#define ENEMY_COUNT 5
#define MAX_LEVEL 5
#define SAVE_FILE "save.txt"
#define ITEM_PER_TYPE 5
#define ITEM_COUNT 15
#define DIALOG_ENDINGS 5

struct Player {
    int row;
    int col;
    int hp;
    int max_hp;
    int attack;
    int defense;
    int level;
    int xp;
};

struct Enemy {
    char name[24];
    int row;
    int col;
    int hp;
    int max_hp;
    int attack;
    int defense;
    int xp_reward;
    int alive;
    int is_boss;
};

enum ItemType {
    WEAPON = 0,
    ARMOR = 1,
    POTION = 2
};

struct Item {
    const char *name;
    enum ItemType type;
    int price;
    int bonus;  /* zbran: utok, brnenie: obrana, elixir: HP */
};

struct GameState {
    struct Player player;
    struct Enemy enemies[ENEMY_COUNT];
    int shrine_used;
    int normal_kills;
    int boss_defeated;
    int quest_shrine_done;
    int quest_hunter_done;
    int quest_escape_done;

    int gold;
    int mana;
    int max_mana;
    int inventory[ITEM_COUNT];
    int equipped_weapon; /* index v katalogu alebo -1 */
    int equipped_armor;
    int dialogue_seen[DIALOG_ENDINGS];
    int dialogue_last;   /* 0 = ziadny, 1..5 = koniec */

    char message[160];
};

extern const struct Item item_catalog[ITEM_COUNT];

void init_game(struct GameState *game);
void gain_xp(struct GameState *game, int amount);
void on_enemy_defeated(struct GameState *game, int enemy_index);

char map_tile(int row, int col);
void print_map(const struct GameState *game);
void move_player(struct GameState *game, int dr, int dc);

int start_combat(struct GameState *game, struct Enemy *enemy);
int save_game(const struct GameState *game, const char *filename);
int load_game(struct GameState *game, const char *filename);

int effective_attack(const struct GameState *game);
int effective_defense(const struct GameState *game);
int item_buy(struct GameState *game, int index);
int item_sell(struct GameState *game, int index);
int item_equip_or_use(struct GameState *game, int index);
int inventory_screen(struct GameState *game, int in_battle);
void shop_screen(struct GameState *game);

int ability_unlocked(const struct GameState *game, int index);
int ability_cost(int index);
int use_ability(struct GameState *game, struct Enemy *enemy, int index,
                int *block_next, char *message, size_t capacity);
int battle_ability_menu(const struct GameState *game);
void skills_screen(const struct GameState *game);

int dialogue_finish(struct GameState *game, int ending);
void dialogue_screen(struct GameState *game);

void terminal_prepare(void);
char read_key(void);

void ui_clear(void);
void ui_header(const char *subtitle);
void ui_wait_key(void);
void ui_main_menu(const char *message);
void ui_world(const struct GameState *game);
void ui_stats(const struct GameState *game);
void ui_quests(const struct GameState *game);
void ui_battle(const struct GameState *game, const struct Enemy *enemy, const char *message);
void ui_end(const struct GameState *game, int victory);

#endif
