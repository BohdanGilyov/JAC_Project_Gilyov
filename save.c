#include "game.h"

#include <stdio.h>
#include <string.h>

#define SAVE_MAGIC_V2 "DUNGEON_ESCAPE_V2"

int save_game(const struct GameState *game, const char *filename)
{
    FILE *file = fopen(filename, "w");
    const struct Player *p = &game->player;
    int ok = 1;
    if (!file) return 0;

    fprintf(file, "%s\n", SAVE_MAGIC_V2);
    fprintf(file, "player %d %d %d %d %d %d %d %d\n",
            p->row, p->col, p->hp, p->max_hp,
            p->attack, p->defense, p->level, p->xp);
    fprintf(file, "progress %d %d %d %d %d %d\n",
            game->shrine_used, game->normal_kills, game->boss_defeated,
            game->quest_shrine_done, game->quest_hunter_done, game->quest_escape_done);
    fprintf(file, "enemies");
    for (int i = 0; i < ENEMY_COUNT; i++) {
        fprintf(file, " %d", game->enemies[i].alive);
    }
    fprintf(file, "\neconomy %d %d %d %d %d %d\n",
            game->gold, game->mana, game->max_mana,
            game->equipped_weapon, game->equipped_armor, game->dialogue_last);
    fprintf(file, "items");
    for (int i = 0; i < ITEM_COUNT; i++) {
        fprintf(file, " %d", game->inventory[i]);
    }
    fprintf(file, "\ndialogue");
    for (int i = 0; i < DIALOG_ENDINGS; i++) {
        fprintf(file, " %d", game->dialogue_seen[i]);
    }
    fprintf(file, "\n");
    if (ferror(file)) ok = 0;

    if (fclose(file) != 0) ok = 0;
    return ok;
}

static int valid_save(const struct GameState *game)
{
    const struct Player *p = &game->player;
    int defeated_normals = 0;
    static const int thresholds[MAX_LEVEL] = {0, 20, 50, 90, 140};

    if (map_tile(p->row, p->col) == '#' ||
        p->level < 1 || p->level > MAX_LEVEL ||
        p->max_hp != 100 + 10 * (p->level - 1) ||
        p->attack != 12 + 2 * (p->level - 1) ||
        p->defense != 3 + p->level - 1 ||
        p->hp < 1 || p->hp > p->max_hp ||
        p->xp < 0 || p->xp > 10000 ||
        game->gold < 0 || game->gold > 100000 ||
        game->max_mana != 20 + 5 * (p->level - 1) ||
        game->mana < 0 || game->mana > game->max_mana) return 0;

    if (p->level < MAX_LEVEL) {
        if (p->xp < thresholds[p->level - 1] || p->xp >= thresholds[p->level])
            return 0;
    } else if (p->xp < thresholds[MAX_LEVEL - 1]) return 0;

    if ((game->shrine_used != 0 && game->shrine_used != 1) ||
        (game->boss_defeated != 0 && game->boss_defeated != 1) ||
        (game->quest_shrine_done != 0 && game->quest_shrine_done != 1) ||
        (game->quest_hunter_done != 0 && game->quest_hunter_done != 1) ||
        (game->quest_escape_done != 0 && game->quest_escape_done != 1) ||
        game->normal_kills < 0 || game->normal_kills > 4 ||
        game->quest_shrine_done != game->shrine_used ||
        game->quest_hunter_done != (game->normal_kills >= 3) ||
        game->boss_defeated != !game->enemies[4].alive ||
        game->quest_escape_done) return 0;

    for (int i = 0; i < ENEMY_COUNT; i++) {
        const struct Enemy *enemy = &game->enemies[i];
        if (enemy->alive != 0 && enemy->alive != 1) return 0;
        if (enemy->alive && enemy->row == p->row && enemy->col == p->col)
            return 0;
        if (!enemy->alive && !enemy->is_boss) defeated_normals++;
    }
    if (defeated_normals != game->normal_kills) return 0;

    for (int i = 0; i < ITEM_COUNT; i++)
        if (game->inventory[i] < 0 || game->inventory[i] > 99) return 0;

    if (game->equipped_weapon != -1) {
        if (game->equipped_weapon < 0 || game->equipped_weapon >= 5 ||
            game->inventory[game->equipped_weapon] < 1) return 0;
    }
    if (game->equipped_armor != -1) {
        if (game->equipped_armor < 5 || game->equipped_armor >= 10 ||
            game->inventory[game->equipped_armor] < 1) return 0;
    }
    if (game->dialogue_last < 0 || game->dialogue_last > DIALOG_ENDINGS)
        return 0;
    for (int i = 0; i < DIALOG_ENDINGS; i++)
        if (game->dialogue_seen[i] != 0 && game->dialogue_seen[i] != 1)
            return 0;
    if (game->dialogue_last && !game->dialogue_seen[game->dialogue_last - 1])
        return 0;
    return 1;
}

static int read_key_word(FILE *file, const char *expected)
{
    char key[24];
    return fscanf(file, " %23s", key) == 1 && strcmp(key, expected) == 0;
}

int load_game(struct GameState *game, const char *filename)
{
    FILE *file = fopen(filename, "r");
    struct GameState loaded;
    char magic[64];
    int alive[ENEMY_COUNT];
    int ok = 1;
    if (!file) return 0;
    init_game(&loaded);

    if (!fgets(magic, sizeof(magic), file)) {
        fclose(file);
        return 0;
    }
    magic[strcspn(magic, "\r\n")] = '\0';
    if (strcmp(magic, SAVE_MAGIC_V2) != 0) {
        fclose(file);
        return 0;
    }

    if (!read_key_word(file, "player") ||
        fscanf(file, "%d %d %d %d %d %d %d %d",
               &loaded.player.row, &loaded.player.col,
               &loaded.player.hp, &loaded.player.max_hp,
               &loaded.player.attack, &loaded.player.defense,
               &loaded.player.level, &loaded.player.xp) != 8 ||
        !read_key_word(file, "progress") ||
        fscanf(file, "%d %d %d %d %d %d",
               &loaded.shrine_used, &loaded.normal_kills,
               &loaded.boss_defeated, &loaded.quest_shrine_done,
               &loaded.quest_hunter_done, &loaded.quest_escape_done) != 6 ||
        !read_key_word(file, "enemies") ||
        fscanf(file, "%d %d %d %d %d",
               &alive[0], &alive[1], &alive[2], &alive[3], &alive[4]) != 5)
        ok = 0;

    if (ok) {
        if (!read_key_word(file, "economy") ||
            fscanf(file, "%d %d %d %d %d %d",
                   &loaded.gold, &loaded.mana, &loaded.max_mana,
                   &loaded.equipped_weapon, &loaded.equipped_armor,
                   &loaded.dialogue_last) != 6 ||
            !read_key_word(file, "items")) ok = 0;
        for (int i = 0; ok && i < ITEM_COUNT; i++)
            if (fscanf(file, "%d", &loaded.inventory[i]) != 1) ok = 0;
        if (ok && !read_key_word(file, "dialogue")) ok = 0;
        for (int i = 0; ok && i < DIALOG_ENDINGS; i++)
            if (fscanf(file, "%d", &loaded.dialogue_seen[i]) != 1) ok = 0;
    }

    if (ok) {
        for (int i = 0; i < ENEMY_COUNT; i++) {
            loaded.enemies[i].alive = alive[i];
            if (!alive[i]) loaded.enemies[i].hp = 0;
        }
        ok = valid_save(&loaded);
    }
    fclose(file);
    if (ok) *game = loaded;
    return ok;
}
