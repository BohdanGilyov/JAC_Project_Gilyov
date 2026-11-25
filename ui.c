#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <unistd.h>
#endif

#ifdef _WIN32
// Zapis riadiace sekvencie priamo do konzoly, nie do presmerovaneho stdout.
static int console_sequence(const char *sequence, DWORD length)
{
    DWORD written;
    return WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), sequence,
                         length, &written, NULL) && written == length;
}
#endif

#ifndef _WIN32
static void restore_screen(void)
{
    printf("\033[?1049l");
    fflush(stdout);
}
#endif

void ui_clear(void)
{
    fflush(stdout);
#ifdef _WIN32
    if (getenv("DUNGEON_STDIN_MODE")) return;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    static int screen_mode = 0;
    DWORD mode = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
    if (screen_mode == 0) {
        if (SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            screen_mode = 1;
        } else {
            screen_mode = 2;
            SetConsoleMode(console, mode);
        }
    }
    // Bez prepinania obrazoviek: vymaz obrazovku aj historiu, kurzor na zaciatok.
    if (screen_mode == 1 && console_sequence("\033[2J\033[3J\033[H", 11)) return;

    // Klasicke WinAPI: vymaz cely buffer a vrat kurzor na zaciatok.
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD start = {0, 0};
    DWORD written;
    if (GetConsoleScreenBufferInfo(console, &info)) {
        DWORD size = (DWORD)info.dwSize.X * info.dwSize.Y;
        if (FillConsoleOutputCharacterA(console, ' ', size, start, &written) &&
            FillConsoleOutputAttribute(console, info.wAttributes, size, start, &written) &&
            SetConsoleCursorPosition(console, start)) return;
    }
    fprintf(stderr, "Nepodarilo sa vymazat konzolu.\n");
    exit(1);
#else
    if (!isatty(STDOUT_FILENO)) return;
    static int alternate_screen = 0;
    if (!alternate_screen) {
        printf("\033[?1049h");
        atexit(restore_screen);
        alternate_screen = 1;
    }
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

void ui_header(const char *subtitle)
{
    puts("===============================================");
    puts("                DUNGEON ESCAPE");
    puts("===============================================");
    printf("%s\n\n", subtitle);
}

void ui_wait_key(void)
{
    puts("\nStlac lubovolnu klavesu pre navrat.");
    read_key();
}

void ui_main_menu(const char *message)
{
    ui_clear();
    ui_header("HLAVNE MENU");
    puts("[N] Nova hra\n[L] Nacitat hru\n[H] Pomoc\n[Q] Koniec");
    printf("\n%s\n", message);
}

static void print_hp(int hp, int max_hp)
{
    printf("HP [");
    for (int i = 0; i < 20; i++) {
        if (i < hp * 20 / max_hp) putchar('#');
        else putchar('-');
    }
    printf("] %d/%d\n", hp, max_hp);
}

void ui_world(const struct GameState *game)
{
    ui_clear();
    ui_header("PODZEMIE");
    print_hp(game->player.hp, game->player.max_hp);
    printf("Level %d/%d | XP %d | Zlato %d | Mana %d/%d\n",
           game->player.level, MAX_LEVEL, game->player.xp,
           game->gold, game->mana, game->max_mana);
    printf("Utok %d | Obrana %d\n\n", effective_attack(game), effective_defense(game));
    print_map(game);
    puts("\nW/A/S/D pohyb | P statistiky | U ulohy | V ulozit");
    puts("I inventar | K schopnosti | O obchod | T dialog");
    puts("H pomoc | Q menu");
    printf("\n%s\n", game->message);
}

void ui_stats(const struct GameState *game)
{
    ui_clear();
    ui_header("STATISTIKY HRACA");
    print_hp(game->player.hp, game->player.max_hp);
    printf("Level: %d/%d | XP: %d\n", game->player.level, MAX_LEVEL, game->player.xp);
    printf("Utok: %d | Obrana: %d\n", effective_attack(game), effective_defense(game));
    printf("Mana: %d/%d | Zlato: %d\n", game->mana, game->max_mana, game->gold);
    printf("Pozicia: %d, %d\n", game->player.row + 1, game->player.col + 1);
    printf("Porazeni nepriatelia: %d\n", game->normal_kills);
    printf("Boss porazeny: %s\n", game->boss_defeated ? "ano" : "nie");
    ui_wait_key();
}

void ui_quests(const struct GameState *game)
{
    ui_clear();
    ui_header("ULOHY");
    printf("1. Najdi svatynu H: %s\n", game->quest_shrine_done ? "SPLNENA" : "NESPLNENA");
    printf("2. Poraz 3 nepriatelov (%d/3): %s\n",
           game->normal_kills < 3 ? game->normal_kills : 3,
           game->quest_hunter_done ? "SPLNENA" : "NESPLNENA");
    printf("3. Boss a vychod X: %s\n", game->quest_escape_done ? "SPLNENA" : "NESPLNENA");
    ui_wait_key();
}

void ui_battle(const struct GameState *game, const struct Enemy *enemy, const char *message)
{
    ui_clear();
    ui_header("BOJ");
    puts("HRAC");
    print_hp(game->player.hp, game->player.max_hp);
    printf("Utok %d | Obrana %d\n\n", effective_attack(game), effective_defense(game));
    printf("NEPRIATEL: %s\n", enemy->name);
    print_hp(enemy->hp, enemy->max_hp);
    printf("Utok %d | Obrana %d\n\n", enemy->attack, enemy->defense);
    puts("[A] utok | [D] obrana");
    printf("%s\n", message);
}

void ui_end(const struct GameState *game, int victory)
{
    ui_clear();
    ui_header(victory ? "VITAZSTVO" : "PREHRA");
    puts(victory ? "Unikol si z podzemia!" : "Hrac prehral boj.");
    printf("Level %d/%d | XP %d | Zlato %d\n", game->player.level, MAX_LEVEL, game->player.xp, game->gold);
    printf("Porazeni bezni nepriatelia: %d\n", game->normal_kills);
    ui_wait_key();
}
