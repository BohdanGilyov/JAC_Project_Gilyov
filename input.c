#include "game.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>

/*
 * CLion Run moze presmerovat standardny vstup/vystup do pipe. Takto
 * program nema skutocnu interaktivnu konzolu a getchar() caka na Enter.
 * Vytvorime preto vlastnu Windows konzolu, nezavislu od Run okna IDE.
 */
void terminal_prepare(void)
{
    if (getenv("DUNGEON_STDIN_MODE") != NULL) {
        return;
    }

    /* Uvolnime pripadnu pseudo-konzolu, aby mala hra vlastne okno. */
    (void)FreeConsole();
    if (!AllocConsole()) {
        /* Ak uz konzola existuje, pokusime sa pouzit ju. */
        if (GetConsoleWindow() == NULL) {
            fputs("Nepodarilo sa vytvorit hernu konzolu.\n", stderr);
            exit(EXIT_FAILURE);
        }
    }

    /* freopen obnovi streamy CRT, ktore CLion mohol presmerovat. */
    if (freopen("CONIN$", "r", stdin) == NULL ||
        freopen("CONOUT$", "w", stdout) == NULL ||
        freopen("CONOUT$", "w", stderr) == NULL) {
        exit(EXIT_FAILURE);
    }

    (void)SetStdHandle(STD_INPUT_HANDLE,
                       (HANDLE)_get_osfhandle(_fileno(stdin)));
    (void)SetStdHandle(STD_OUTPUT_HANDLE,
                       (HANDLE)_get_osfhandle(_fileno(stdout)));
    (void)SetStdHandle(STD_ERROR_HANDLE,
                       (HANDLE)_get_osfhandle(_fileno(stderr)));
    (void)SetConsoleTitleA("Dungeon Escape - JAC Project");

    HWND window = GetConsoleWindow();
    if (window != NULL) {
        (void)ShowWindow(window, SW_SHOW);
        (void)SetForegroundWindow(window);
    }
}

/*
 * ReadConsoleInputW vracia udalosti klavesnice hned pri stlaceni (bez
 * Enter). Nezobrazuje stlacene pismena a nepracuje s CLion stdin pipe.
 * Drzanie klavesu nezopakuje pohyb, az kym sa klaves nepusti.
 */
static char read_console_key(void)
{
    static unsigned char pressed[256] = {0};
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;

    if (input == NULL || input == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(input, &mode)) {
        return 0;
    }

    for (;;) {
        INPUT_RECORD event;
        DWORD read_count = 0;
        if (!ReadConsoleInputW(input, &event, 1, &read_count) ||
            read_count != 1) {
            return 0;
        }
        if (event.EventType != KEY_EVENT) {
            continue;
        }

        KEY_EVENT_RECORD key = event.Event.KeyEvent;
        unsigned int vk = key.wVirtualKeyCode;
        if (!key.bKeyDown) {
            if (vk < 256) {
                pressed[vk] = 0;
            }
            continue;
        }
        if (vk < 256 && pressed[vk]) {
            continue;
        }
        if (vk < 256) {
            pressed[vk] = 1;
        }
        if (key.uChar.UnicodeChar > 0 && key.uChar.UnicodeChar <= 127) {
            return (char)tolower((unsigned char)key.uChar.UnicodeChar);
        }
    }
}
#else
#include <termios.h>
#include <unistd.h>

void terminal_prepare(void)
{
    /* POSIX terminaly uz podporuju nekanonicky rezim cez termios. */
}
#endif

char read_key(void)
{
    ui_present();
#ifdef _WIN32
    /* Vyhradne pre automaticke skriptovane testy s presmerovanym stdin. */
    if (getenv("DUNGEON_STDIN_MODE") != NULL) {
        int ch = getchar();
        return ch == EOF ? 0 : (char)tolower((unsigned char)ch);
    }
    return read_console_key();
#else
    int ch;
    if (!isatty(STDIN_FILENO)) {
        ch = getchar();
    } else {
        struct termios original;
        struct termios raw;

        if (tcgetattr(STDIN_FILENO, &original) != 0) {
            return 0;
        }
        raw = original;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
            return 0;
        }
        ch = getchar();
        (void)tcsetattr(STDIN_FILENO, TCSANOW, &original);
    }
    return ch == EOF ? 0 : (char)tolower((unsigned char)ch);
#endif
}
