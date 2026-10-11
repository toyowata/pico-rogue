#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "curses.h"
#include "extern.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "hardware/sync.h"

void md_init(void) {
    stdio_init_all();

    // Wait briefly for USB CDC connection
    sleep_ms(1000);

    // Initial prompt so player sees the game has booted over serial
    printf("\033[0m\033[2J\033[H");
    printf("\r\n======================================================\r\n");
    printf("        Rogue 5.4 - Raspberry Pi Pico Port             \r\n");
    printf("======================================================\r\n");
    printf("Serial Configuration: 115200 8N1 (ANSI / VT100)\r\n");
    printf("Terminal recommended size: 80 cols x 24 lines\r\n");
    printf("\r\nControls:\r\n");
    printf("  Arrow keys or hjkl / yubn: Move\r\n");
    printf("  Shift + Arrow keys       : Run / Fast Move\r\n");
    printf("  ?                        : In-game help\r\n");
    printf("  Q                        : Quit game\r\n");
    printf("\r\nPress [SPACE] or [RETURN] to enter Dungeons of Doom...\r\n");
    fflush(stdout);

    // Wait for player to press a key before starting
    while (1) {
        int c = getchar_timeout_us(100000);
        if (c >= 0) {
            break;
        }
    }
}

int md_readchar(void) {
    return getch();
}

void md_sleep(int s) {
    sleep_ms(s * 1000);
}

int md_getpid(void) {
    // Use microseconds since boot as unique pseudo-PID and random entropy
    return (int)to_us_since_boot(get_absolute_time());
}

int md_getuid(void) {
    return 1000;
}

char *md_getusername(void) {
    return "Rodney";
}

char *md_gethomedir(void) {
    return "";
}

int md_erasechar(void) {
    return '\b';
}

int md_killchar(void) {
    return 0x15; /* Ctrl-U */
}

int md_suspchar(void) {
    return 0x1a; /* Ctrl-Z */
}

int md_dsuspchar(void) {
    return 0x19; /* Ctrl-Y */
}

int md_setdsuspchar(int c) {
    (void)c;
    return 0;
}

int md_shellescape(void) {
    return 0;
}

int md_chmod(char *filename, int mode) {
    (void)filename;
    (void)mode;
    return 0;
}

int md_unlink(char *file) {
    (void)file;
    return 0;
}

int md_unlink_open_file(char *file, FILE *inf) {
    (void)file;
    (void)inf;
    return 0;
}

void md_raw_standout(void) {
    printf("\033[7m");
    fflush(stdout);
}

void md_raw_standend(void) {
    printf("\033[0m");
    fflush(stdout);
}

int md_hasclreol(void) {
    return 1;
}

int md_issymlink(char *sp) {
    (void)sp;
    return 0;
}

void md_normaluser(void) {}
void md_tstpsignal(void) {}
void md_tstphold(void) {}
void md_tstpresume(void) {}
void md_ignoreallsignals(void) {}
void md_onsignal_autosave(void) {}
void md_onsignal_exit(void) {}
void md_onsignal_default(void) {}

char *md_crypt(char *key, char *salt) {
    (void)key;
    (void)salt;
    return "";
}

char *md_getpass(char *prompt) {
    (void)prompt;
    return "";
}

char *md_getrealname(int uid) {
    (void)uid;
    return "Rodney";
}
