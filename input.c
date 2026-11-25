#include "game.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

void terminal_prepare(void)
{
    if (getenv("DUNGEON_STDIN_MODE")) return;
    // Vzdy otvor vlastnu konzolu, aj ked hru spusta CLion.
    FreeConsole();
    if (!AllocConsole()) {
        fprintf(stderr, "Nepodarilo sa otvorit konzolu.\n");
        exit(1);
    }
    // Presmeruj standardne prudy do novej konzoly.
    if (!freopen("CONIN$", "r", stdin) ||
        !freopen("CONOUT$", "w", stdout) ||
        !freopen("CONOUT$", "w", stderr)) {
        exit(1);
    }
    SetStdHandle(STD_INPUT_HANDLE, (HANDLE)_get_osfhandle(_fileno(stdin)));
    // WinAPI potrebuje konzolovy handle s pravom citat aj zapisovat.
    HANDLE output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL, OPEN_EXISTING, 0, NULL);
    if (output == INVALID_HANDLE_VALUE ||
        !SetStdHandle(STD_OUTPUT_HANDLE, output) ||
        !SetStdHandle(STD_ERROR_HANDLE, output) ||
        !SetConsoleMode(output, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT)) {
        fprintf(stderr, "Nepodarilo sa nastavit konzolu.\n");
        exit(1);
    }
    // Prevod \n na CRLF vo Windows.
    _setmode(_fileno(stdout), _O_TEXT);
    _setmode(_fileno(stderr), _O_TEXT);
    SetConsoleTitleA("Dungeon Escape");
}

char read_key(void)
{
    fflush(stdout);
    int key;
    if (getenv("DUNGEON_STDIN_MODE")) {
        key = getchar();
    } else {
        do {
            key = _getch();
            if (key == 0 || key == 224) {
                _getch();
                key = '\r';
            }
        } while (key == '\r' || key == '\n');
    }
    if (key == EOF) return 0;
    return tolower(key);
}
#else
#include <termios.h>
#include <unistd.h>

void terminal_prepare(void)
{
}

char read_key(void)
{
    fflush(stdout);
    int key;
    if (isatty(STDIN_FILENO)) {
        struct termios old;
        if (tcgetattr(STDIN_FILENO, &old) != 0) return 0;
        struct termios mode = old;
        mode.c_lflag &= ~(ICANON | ECHO);
        mode.c_cc[VMIN] = 1;
        mode.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &mode) != 0) return 0;
        key = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
    } else {
        key = getchar();
    }
    if (key == EOF) return 0;
    return tolower(key);
}
#endif
