#include "game.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* Vsetky obrazovky sa skladaju v RAM. Ziadna herna funkcia nepise
   jednotlivy riadok do terminalu. */
#define FRAME_SIZE 16384
static char frame[FRAME_SIZE];
static size_t frame_length;
static int redraw_enabled;

#ifdef _WIN32
static HANDLE console_output = INVALID_HANDLE_VALUE;
static DWORD original_mode;
static int original_mode_valid;
static int test_mode;
#endif

void ui_printf(const char *format, ...)
{
    if (frame_length >= FRAME_SIZE - 1) return;
    va_list args;
    va_start(args, format);
    size_t left = FRAME_SIZE - frame_length;
    int n = vsnprintf(frame + frame_length, left, format, args);
    va_end(args);
    if (n < 0) return;
    if ((size_t)n >= left) frame_length = FRAME_SIZE - 1;
    else frame_length += (size_t)n;
}

void ui_puts(const char *value)
{
    ui_printf("%s\n", value);
}

void ui_putchar(int character)
{
    if (frame_length + 1 < FRAME_SIZE) {
        frame[frame_length++] = (char)character;
        frame[frame_length] = '\0';
    }
}

#ifdef _WIN32
static void console_write(const char *value, size_t length)
{
    while (length != 0) {
        DWORD written = 0;
        DWORD count = length > 8192 ? 8192 : (DWORD)length;
        if (!WriteConsoleA(console_output, value, count, &written, NULL) ||
            written == 0) {
            DWORD error_code = GetLastError();
            fprintf(stderr, "CHYBA: Windows Terminal nevie prekreslit hru "
                            "(WriteConsoleA, kod %lu).\n",
                    (unsigned long)error_code);
            exit(EXIT_FAILURE);
        }
        length -= written;
        value += written;
    }
}
#endif

static void ui_shutdown(void)
{
    if (redraw_enabled) {
#ifdef _WIN32
        const char reset[] = "\x1b[0m\x1b[?25h\x1b[?1049l";
        DWORD written = 0;
        (void)WriteConsoleA(console_output, reset, (DWORD)(sizeof(reset) - 1),
                            &written, NULL);
        if (original_mode_valid)
            (void)SetConsoleMode(console_output, original_mode);
        if (console_output != INVALID_HANDLE_VALUE)
            (void)CloseHandle(console_output);
#else
        fputs("\x1b[0m\x1b[?25h\x1b[?1049l", stdout);
        fflush(stdout);
#endif
    }
}

void ui_init(void)
{
    terminal_prepare();
    setvbuf(stdout, NULL, _IONBF, 0);
#ifdef _WIN32
    /* Vzdy pouzijeme vlastny konzolovy vystup, nie pipe z CLion. */
    test_mode = getenv("DUNGEON_STDIN_MODE") != NULL;
    if (!test_mode) {
        console_output = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (console_output == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "CHYBA: CONOUT$ sa nepodarilo otvorit.\n");
            exit(EXIT_FAILURE);
        }
        if (!GetConsoleMode(console_output, &original_mode)) {
            fprintf(stderr, "CHYBA: konzolovy vystup nie je terminal.\n");
            exit(EXIT_FAILURE);
        }
        DWORD vt_mode = original_mode | ENABLE_PROCESSED_OUTPUT |
                                         ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if (!SetConsoleMode(console_output, vt_mode)) {
            fprintf(stderr, "CHYBA: Windows terminal nepodporuje VT rezim (kod %lu).\n",
                    (unsigned long)GetLastError());
            exit(EXIT_FAILURE);
        }
        original_mode_valid = 1;
        redraw_enabled = 1;
        const char start[] = "\x1b[?1049h\x1b[?25l\x1b[H\x1b[2J";
        console_write(start, sizeof(start) - 1);
    }
#else
    if (isatty(STDOUT_FILENO)) {
        redraw_enabled = 1;
        fputs("\x1b[?1049h\x1b[?25l\x1b[H\x1b[2J", stdout);
    }
#endif
    (void)atexit(ui_shutdown);
}

void ui_clear(void)
{
    frame_length = 0;
    frame[0] = '\0';
}

/* Kazdy pohyb: jedna sekvencia Home + Erase, nasleduje JEDEN novy
   obraz. Ziadny WriteConsoleOutputW, ziadne fwrite ako nahradna
   cesta v skutocnom Windows terminali. Microsoft podporuje tieto VT
   sekvencie pre Windows Terminal a pre conhost vo VT rezime. */
void ui_present(void)
{
    static const char clear_home[] = "\x1b[H\x1b[2J";
    size_t content_length = frame_length;
    while (content_length && (frame[content_length - 1] == '\n' ||
                              frame[content_length - 1] == '\r')) {
        --content_length;
    }
#ifdef _WIN32
    if (!test_mode) {
        char output[FRAME_SIZE + sizeof(clear_home)];
        const size_t prefix_size = sizeof(clear_home) - 1;
        memcpy(output, clear_home, prefix_size);
        memcpy(output + prefix_size, frame, content_length);
        console_write(output, prefix_size + content_length);
        return;
    }
#else
    if (redraw_enabled) {
        fputs(clear_home, stdout);
    }
#endif
    (void)fwrite(frame, 1, content_length, stdout);
    (void)fflush(stdout);
}


/* 61-character frames keep the map readable in a normal Windows console. */
static void ui_line(void)
{
    ui_puts("+===========================================================+");
}

static void ui_subline(void)
{
    ui_puts("+-----------------------------------------------------------+");
}

static void ui_box_line(const char *value)
{
    ui_printf("| %-57.57s |\n", value ? value : "");
}

static void ui_box_linef(const char *format, ...)
{
    char text[256];
    va_list args;
    va_start(args, format);
    vsnprintf(text, sizeof(text), format, args);
    va_end(args);
    ui_box_line(text);
}

static void ui_section(const char *name)
{
    ui_box_linef("--- %s ---", name);
}

static void make_hp_bar(char *bar, size_t capacity, int hp, int max_hp)
{
    const int width = 20;
    int fill = max_hp > 0 ? hp * width / max_hp : 0;
    if (capacity < (size_t)width + 1) return;
    if (fill < 0) fill = 0;
    if (fill > width) fill = width;
    for (int i = 0; i < width; ++i) {
        bar[i] = (i < fill) ? '#' : '-';
    }
    bar[width] = '\0';
}

/* Map drawn inside its own framed section, without duplicate legend.
   The map logic in map.c is not changed. */
static void ui_map_in_box(const GameState *game)
{
    ui_box_line("          1  2  3  4  5  6  7  8");
    for (int row = 0; row < MAP_ROWS; ++row) {
        char line[96];
        int pos = snprintf(line, sizeof(line), "     %d    ", row + 1);
        for (int col = 0; col < MAP_COLS; ++col) {
            char tile = map_tile(row, col);
            for (int i = 0; i < ENEMY_COUNT; ++i) {
                const Enemy *e = &game->enemies[i];
                if (e->alive && e->row == row && e->col == col) {
                    tile = e->is_boss ? 'B' : 'E';
                }
            }
            if (row == 3 && col == 3 && game->shrine_used) tile = 'h';
            if (game->player.row == row && game->player.col == col) tile = '@';
            if (pos >= 0 && (size_t)pos < sizeof(line)) {
                int count = snprintf(line + pos, sizeof(line) - (size_t)pos,
                                     "%c  ", tile);
                if (count > 0) pos += count;
            }
        }
        ui_box_line(line);
    }
}

void ui_header(const char *subtitle)
{
    ui_line();
    ui_box_line("                      DUNGEON ESCAPE");
    ui_subline();
    if (subtitle && subtitle[0]) ui_box_line(subtitle);
}

void ui_wait_key(void)
{
    ui_box_line("Stlac lubovolnu klavesu pre navrat (bez Enter).");
    ui_line();
    (void)read_key();
}

void ui_main_menu(const char *message)
{
    ui_clear();
    ui_header("HLAVNE MENU");
    ui_section("VOLBY");
    ui_box_line("[N] Nova hra       [L] Nacitat hru");
    ui_box_line("[H] Pomoc          [Q] Koniec");
    ui_section("INFO");
    ui_box_line(message ? message : "Vyber moznost bez Enter.");
    ui_line();
}

void ui_world(const GameState *game)
{
    char bar[32];
    make_hp_bar(bar, sizeof(bar), game->player.hp, game->player.max_hp);
    ui_clear();
    ui_header("PODZEMIE  |  ULOHY: 3  |  NEPRIATELIA: 5");

    ui_section("STAV HRACA");
    ui_box_linef("HP [%s] %d/%d", bar, game->player.hp, game->player.max_hp);
    ui_box_linef("LEVEL %d/%d | XP %d | UTOK %d | OBRANA %d",
                 game->player.level, MAX_LEVEL, game->player.xp,
                 effective_attack(game), effective_defense(game));
    ui_box_linef("ZLATO %d | MANA %d/%d", game->gold,
                 game->mana, game->max_mana);
    ui_subline();

    ui_section("MAPA");
    ui_map_in_box(game);
    ui_subline();

    ui_section("LEGENDA");
    ui_box_line("@ hrac   # stena   E nepriatel   B boss");
    ui_box_line("H svatyna   h pouzita   X vychod   M obchod   N mudrc");
    ui_subline();

    ui_section("OVLADANIE");
    ui_box_line("W/A/S/D pohyb  P hrac  U ulohy  V save  Q menu");
    ui_box_line("I inventar  K schopnosti  O obchod  T dialog  H pomoc");
    ui_subline();

    ui_section("INFO");
    ui_box_line(game->message);
    ui_line();
}

void ui_stats(const GameState *game)
{
    ui_clear();
    ui_header("STATISTIKY HRACA");
    ui_section("POSTAVA");
    ui_box_linef("Pozicia: riadok %d, stlpec %d",
                 game->player.row + 1, game->player.col + 1);
    ui_box_linef("HP: %d/%d | Mana: %d/%d",
                 game->player.hp, game->player.max_hp,
                 game->mana, game->max_mana);
    ui_box_linef("Level: %d/%d | XP: %d | Zlato: %d",
                 game->player.level, MAX_LEVEL, game->player.xp, game->gold);
    ui_box_linef("Utok: %d | Obrana: %d",
                 effective_attack(game), effective_defense(game));
    ui_box_linef("Porazeni nepriatelia: %d", game->normal_kills);
    ui_box_linef("Boss porazeny: %s", game->boss_defeated ? "ano" : "nie");
    ui_box_line("Level-up zvysuje max HP, nie aktualne HP.");
    ui_wait_key();
}

void ui_quests(const GameState *game)
{
    ui_clear();
    ui_header("DENNIK ULOH");
    ui_section("TRI ULOHY");
    ui_box_linef("1. Najdi svatynu H: %s",
                 game->quest_shrine_done ? "SPLNENA" : "NESPLNENA");
    ui_box_linef("2. Poraz 3 nepriatelov (%d/3): %s",
                 game->normal_kills < 3 ? game->normal_kills : 3,
                 game->quest_hunter_done ? "SPLNENA" : "NESPLNENA");
    ui_box_linef("3. Boss a vychod X: %s",
                 game->quest_escape_done ? "SPLNENA" : "NESPLNENA");
    ui_wait_key();
}

void ui_help(void)
{
    ui_clear();
    ui_header("POMOC");
    ui_section("MAPA");
    ui_box_line("W/A/S/D pohyb | P hrac | U ulohy | V save");
    ui_box_line("I inventar | K schopnosti | H pomoc | Q menu");
    ui_box_line("O obchod na M | T dialog na N");
    ui_section("BOJ");
    ui_box_line("A utok | D obrana | K schopnosti | I inventar");
    ui_section("CIEL");
    ui_box_line("Najdi H, poraz 3 nepriatelov a bossa, potom chod na X.");
    ui_wait_key();
}

void ui_battle(const GameState *game, const Enemy *enemy, const char *message)
{
    char bar[32];
    ui_clear();
    ui_header("BOJ - STRIEDANIE TAHOV");
    ui_section("HRAC");
    make_hp_bar(bar, sizeof(bar), game->player.hp, game->player.max_hp);
    ui_box_linef("LEVEL %d | HP [%s] %d/%d", game->player.level,
                 bar, game->player.hp, game->player.max_hp);
    ui_box_linef("UTOK %d | OBRANA %d | MANA %d/%d",
                 effective_attack(game), effective_defense(game),
                 game->mana, game->max_mana);
    ui_subline();
    ui_section("NEPRIATEL");
    make_hp_bar(bar, sizeof(bar), enemy->hp, enemy->max_hp);
    ui_box_linef("%s | HP [%s] %d/%d", enemy->name, bar,
                 enemy->hp, enemy->max_hp);
    ui_box_linef("UTOK %d | OBRANA %d", enemy->attack, enemy->defense);
    ui_subline();
    ui_section("TVOJ TAH");
    ui_box_line("[A] utok    [D] obrana    [K] schopnosti    [I] inventar");
    ui_subline();
    ui_section("INFO");
    ui_box_line(message ? message : "");
    ui_line();
}

void ui_end(const GameState *game, int victory)
{
    ui_clear();
    ui_header(victory ? "VITAZSTVO" : "PREHRA");
    ui_section("KONIEC HRY");
    ui_box_line(victory ? "Unikol si z podzemia!" : "Hrac prehral boj.");
    ui_box_linef("Level %d/%d | XP %d | Zlato %d",
                 game->player.level, MAX_LEVEL, game->player.xp, game->gold);
    ui_box_linef("Porazeni bezni nepriatelia: %d", game->normal_kills);
    ui_box_linef("Boss porazeny: %s", game->boss_defeated ? "ano" : "nie");
    ui_wait_key();
}
