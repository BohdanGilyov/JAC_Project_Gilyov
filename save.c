#include "game.h"

#include <stdio.h>
#include <string.h>

#define SAVE_MAGIC "DUNGEON_ESCAPE_V1"

int save_game(const GameState *game, const char *filename)
{
    FILE *file = fopen(filename, "w");
    const Player *p = &game->player;
    int ok = 1;

    if (!file) {
        return 0;
    }

    if (fprintf(file, "%s\n", SAVE_MAGIC) < 0 ||
        fprintf(file, "player %d %d %d %d %d %d %d %d\n",
                p->row, p->col, p->hp, p->max_hp,
                p->attack, p->defense, p->level, p->xp) < 0 ||
        fprintf(file, "progress %d %d %d %d %d %d\n",
                game->shrine_used, game->normal_kills,
                game->boss_defeated, game->quest_shrine_done,
                game->quest_hunter_done, game->quest_escape_done) < 0 ||
        fprintf(file, "enemies %d %d %d %d %d\n",
                game->enemies[0].alive, game->enemies[1].alive,
                game->enemies[2].alive, game->enemies[3].alive,
                game->enemies[4].alive) < 0) {
        ok = 0;
    }

    if (fclose(file) != 0) {
        ok = 0;
    }
    return ok;
}

static int valid_save(const GameState *game)
{
    const Player *p = &game->player;
    int defeated_normals = 0;

    if (map_tile(p->row, p->col) == '#' ||
        p->level < 1 || p->level > MAX_LEVEL ||
        p->max_hp != 100 + 10 * (p->level - 1) ||
        p->attack != 12 + 2 * (p->level - 1) ||
        p->defense != 3 + (p->level - 1) ||
        p->hp < 1 || p->hp > p->max_hp ||
        p->xp < 0 || p->xp > 10000) {
        return 0;
    }

    if (p->level < MAX_LEVEL) {
        static const int thresholds[MAX_LEVEL] = {0, 20, 50, 90, 140};
        if (p->xp < thresholds[p->level - 1] || p->xp >= thresholds[p->level]) {
            return 0;
        }
    } else if (p->xp < 140) {
        return 0;
    }

    if ((game->shrine_used != 0 && game->shrine_used != 1) ||
        (game->boss_defeated != 0 && game->boss_defeated != 1) ||
        (game->quest_shrine_done != 0 && game->quest_shrine_done != 1) ||
        (game->quest_hunter_done != 0 && game->quest_hunter_done != 1) ||
        (game->quest_escape_done != 0 && game->quest_escape_done != 1) ||
        game->normal_kills < 0 || game->normal_kills > 4 ||
        game->quest_shrine_done != game->shrine_used ||
        game->quest_hunter_done != (game->normal_kills >= 3) ||
        game->boss_defeated != !game->enemies[4].alive ||
        game->quest_escape_done) {
        return 0;
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        const Enemy *enemy = &game->enemies[i];
        if (enemy->alive != 0 && enemy->alive != 1) {
            return 0;
        }
        if (enemy->alive && enemy->row == p->row && enemy->col == p->col) {
            return 0;
        }
        if (!enemy->alive) {
            if (!enemy->is_boss) {
                defeated_normals++;
            }
        }
    }
    return defeated_normals == game->normal_kills;
}

int load_game(GameState *game, const char *filename)
{
    FILE *file = fopen(filename, "r");
    GameState loaded;
    char magic[64];
    char key[16];
    int alive[ENEMY_COUNT];
    int ok = 1;

    if (!file) {
        return 0;
    }
    init_game(&loaded);

    if (!fgets(magic, sizeof(magic), file) ||
        strcspn(magic, "\r\n") != strlen(SAVE_MAGIC) ||
        strncmp(magic, SAVE_MAGIC, strlen(SAVE_MAGIC)) != 0) {
        ok = 0;
    }

    if (ok && (fscanf(file, " %15s", key) != 1 || strcmp(key, "player") != 0 ||
               fscanf(file, "%d %d %d %d %d %d %d %d",
                      &loaded.player.row, &loaded.player.col,
                      &loaded.player.hp, &loaded.player.max_hp,
                      &loaded.player.attack, &loaded.player.defense,
                      &loaded.player.level, &loaded.player.xp) != 8)) {
        ok = 0;
    }

    if (ok && (fscanf(file, " %15s", key) != 1 || strcmp(key, "progress") != 0 ||
               fscanf(file, "%d %d %d %d %d %d",
                      &loaded.shrine_used, &loaded.normal_kills,
                      &loaded.boss_defeated, &loaded.quest_shrine_done,
                      &loaded.quest_hunter_done, &loaded.quest_escape_done) != 6)) {
        ok = 0;
    }

    if (ok && (fscanf(file, " %15s", key) != 1 || strcmp(key, "enemies") != 0 ||
               fscanf(file, "%d %d %d %d %d",
                      &alive[0], &alive[1], &alive[2], &alive[3], &alive[4]) != 5)) {
        ok = 0;
    }

    if (ok) {
        for (int i = 0; i < ENEMY_COUNT; i++) {
            loaded.enemies[i].alive = alive[i];
            if (!alive[i]) {
                loaded.enemies[i].hp = 0;
            }
        }
        ok = valid_save(&loaded);
    }

    fclose(file);
    if (ok) {
        *game = loaded;
    }
    return ok;
}
