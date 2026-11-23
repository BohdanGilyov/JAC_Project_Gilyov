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

typedef struct {
    int row;
    int col;
    int hp;
    int max_hp;
    int attack;
    int defense;
    int level;
    int xp;
} Player;

typedef struct {
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
} Enemy;

typedef enum {
    WEAPON = 0,
    ARMOR = 1,
    POTION = 2
} ItemType;

typedef struct {
    const char *name;
    ItemType type;
    int price;
    int bonus;  /* zbran: utok, brnenie: obrana, elixir: HP */
} Item;

typedef struct {
    Player player;
    Enemy enemies[ENEMY_COUNT];
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
} GameState;

extern const Item item_catalog[ITEM_COUNT];

void init_game(GameState *game);
void game_message(GameState *game, const char *format, ...);
void gain_xp(GameState *game, int amount);
void on_enemy_defeated(GameState *game, int enemy_index);

char map_tile(int row, int col);
void print_map(const GameState *game);
void move_player(GameState *game, int dr, int dc);

int start_combat(GameState *game, Enemy *enemy);
int save_game(const GameState *game, const char *filename);
int load_game(GameState *game, const char *filename);

int effective_attack(const GameState *game);
int effective_defense(const GameState *game);
int item_buy(GameState *game, int index);
int item_sell(GameState *game, int index);
int item_equip_or_use(GameState *game, int index);
int inventory_screen(GameState *game, int in_battle);
void shop_screen(GameState *game);

int ability_unlocked(const GameState *game, int index);
int ability_cost(int index);
int use_ability(GameState *game, Enemy *enemy, int index,
                int *block_next, char *message, size_t capacity);
int battle_ability_menu(const GameState *game);
void skills_screen(const GameState *game);

int dialogue_finish(GameState *game, int ending);
void dialogue_screen(GameState *game);

void terminal_prepare(void);
char read_key(void);

void ui_init(void);
void ui_present(void);
void ui_printf(const char *format, ...);
void ui_puts(const char *text);
void ui_putchar(int symbol);
void ui_clear(void);
void ui_header(const char *subtitle);
void ui_wait_key(void);
void ui_main_menu(const char *message);
void ui_world(const GameState *game);
void ui_stats(const GameState *game);
void ui_quests(const GameState *game);
void ui_help(void);
void ui_battle(const GameState *game, const Enemy *enemy, const char *message);
void ui_end(const GameState *game, int victory);

#endif
