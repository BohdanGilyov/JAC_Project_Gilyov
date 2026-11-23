#ifndef GAME_H
#define GAME_H

#define MAP_ROWS 8
#define MAP_COLS 8
#define ENEMY_COUNT 5
#define MAX_LEVEL 5
#define SAVE_FILE "save.txt"

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
    int attack;
    int defense;
    int xp_reward;
    int alive;
    int is_boss;
} Enemy;

typedef struct {
    Player player;
    Enemy enemies[ENEMY_COUNT];
    int shrine_used;
    int normal_kills;
    int boss_defeated;
    int quest_shrine_done;
    int quest_hunter_done;
    int quest_escape_done;
} GameState;

void init_game(GameState *game);
void show_player(const GameState *game);
void show_quests(const GameState *game);
void gain_xp(GameState *game, int amount);
void on_enemy_defeated(GameState *game, int enemy_index);

char map_tile(int row, int col);
void print_map(const GameState *game);
void move_player(GameState *game, int dr, int dc);

int start_combat(GameState *game, Enemy *enemy);

int save_game(const GameState *game, const char *filename);
int load_game(GameState *game, const char *filename);

char read_command(void);

#endif
