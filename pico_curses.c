/*
 * pico_curses.c - Curses implementation over ANSI VT100 USB Serial for RP2040
 */

#include "curses.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static WINDOW _stdscr_inst;
static WINDOW _curscr_inst;
static WINDOW _hw_inst;

WINDOW *stdscr = &_stdscr_inst;
WINDOW *curscr = &_curscr_inst;

static bool g_clearok = false;

static void init_window_struct(WINDOW *win, int nlines, int ncols, int begy, int begx) {
    win->_cury = 0;
    win->_curx = 0;
    win->_maxy = (nlines > LINES) ? LINES : nlines;
    win->_maxx = (ncols > COLS) ? COLS : ncols;
    win->_begy = begy;
    win->_begx = begx;
    win->_flags = 0;
    win->_attrs = A_NORMAL;
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            win->_lines[y][x] = ' ';
        }
    }
}

WINDOW *initscr(void) {
    init_window_struct(stdscr, LINES, COLS, 0, 0);
    init_window_struct(curscr, LINES, COLS, 0, 0);
    init_window_struct(&_hw_inst, LINES, COLS, 0, 0);
    hw = &_hw_inst;

    // Invalidate curscr buffer so initial refresh draws everything
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            curscr->_lines[y][x] = 0xFFFF;
        }
    }

    g_clearok = true;

    // ANSI terminal reset and clear
    printf("\033[0m\033[2J\033[H\033[?25h");
    fflush(stdout);

    return stdscr;
}

int endwin(void) {
    printf("\033[%d;1H\033[0m\033[?25h\r\n", LINES);
    fflush(stdout);
    return OK;
}

WINDOW *newwin(int nlines, int ncols, int begy, int begx) {
    WINDOW *win = (WINDOW *)malloc(sizeof(WINDOW));
    if (!win) return NULL;
    init_window_struct(win, nlines, ncols, begy, begx);
    return win;
}

WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begy, int begx) {
    (void)orig;
    return newwin(nlines, ncols, begy, begx);
}

int delwin(WINDOW *win) {
    if (win && win != stdscr && win != curscr && win != &_hw_inst) {
        free(win);
    }
    return OK;
}

int wmove(WINDOW *win, int y, int x) {
    if (!win) win = stdscr;
    if (y < 0) y = 0;
    if (y >= LINES) y = LINES - 1;
    if (x < 0) x = 0;
    if (x >= COLS) x = COLS - 1;

    win->_cury = y;
    win->_curx = x;
    return OK;
}

int move(int y, int x) {
    return wmove(stdscr, y, x);
}

int waddch(WINDOW *win, chtype ch) {
    if (!win) win = stdscr;

    char c = (char)(ch & A_CHARTEXT);

    if (c == '\n') {
        win->_curx = 0;
        if (win->_cury < LINES - 1) win->_cury++;
        return OK;
    }
    if (c == '\r') {
        win->_curx = 0;
        return OK;
    }
    if (c == '\b') {
        if (win->_curx > 0) win->_curx--;
        return OK;
    }
    if (c == '\t') {
        int next_tab = (win->_curx + 8) & ~7;
        while (win->_curx < next_tab && win->_curx < COLS) {
            waddch(win, ' ');
        }
        return OK;
    }

    if (win->_cury >= 0 && win->_cury < LINES && win->_curx >= 0 && win->_curx < COLS) {
        win->_lines[win->_cury][win->_curx] = (ch & A_CHARTEXT) | win->_attrs | (ch & ~A_CHARTEXT);
        win->_curx++;
        if (win->_curx >= COLS) {
            win->_curx = 0;
            if (win->_cury < LINES - 1) win->_cury++;
        }
    }
    return OK;
}

int addch(chtype ch) {
    return waddch(stdscr, ch);
}

int mvaddch(int y, int x, chtype ch) {
    if (wmove(stdscr, y, x) == ERR) return ERR;
    return waddch(stdscr, ch);
}

int mvwaddch(WINDOW *win, int y, int x, chtype ch) {
    if (wmove(win, y, x) == ERR) return ERR;
    return waddch(win, ch);
}

int waddstr(WINDOW *win, const char *str) {
    if (!win) win = stdscr;
    if (!str) return ERR;
    while (*str) {
        waddch(win, (unsigned char)*str++);
    }
    return OK;
}

int addstr(const char *str) {
    return waddstr(stdscr, str);
}

int mvaddstr(int y, int x, const char *str) {
    if (wmove(stdscr, y, x) == ERR) return ERR;
    return waddstr(stdscr, str);
}

int wprintw(WINDOW *win, const char *fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return waddstr(win, buf);
}

int printw(const char *fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return waddstr(stdscr, buf);
}

int mvprintw(int y, int x, const char *fmt, ...) {
    if (wmove(stdscr, y, x) == ERR) return ERR;
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return waddstr(stdscr, buf);
}

int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...) {
    if (wmove(win, y, x) == ERR) return ERR;
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return waddstr(win, buf);
}

chtype winch(WINDOW *win) {
    if (!win) win = stdscr;
    if (win->_cury >= 0 && win->_cury < LINES && win->_curx >= 0 && win->_curx < COLS) {
        return win->_lines[win->_cury][win->_curx];
    }
    return ' ';
}

int werase(WINDOW *win) {
    if (!win) win = stdscr;
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            win->_lines[y][x] = ' ';
        }
    }
    win->_cury = 0;
    win->_curx = 0;
    return OK;
}

int erase(void) {
    return werase(stdscr);
}

int wclear(WINDOW *win) {
    clearok(curscr, TRUE);
    return werase(win);
}

int clear(void) {
    return wclear(stdscr);
}

int wclrtoeol(WINDOW *win) {
    if (!win) win = stdscr;
    int y = win->_cury;
    if (y >= 0 && y < LINES) {
        for (int x = win->_curx; x < COLS; x++) {
            win->_lines[y][x] = ' ';
        }
    }
    return OK;
}

int clrtoeol(void) {
    return wclrtoeol(stdscr);
}

int wclrtobot(WINDOW *win) {
    if (!win) win = stdscr;
    wclrtoeol(win);
    for (int y = win->_cury + 1; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            win->_lines[y][x] = ' ';
        }
    }
    return OK;
}

int clrtobot(void) {
    return wclrtobot(stdscr);
}

int wstandout(WINDOW *win) {
    if (!win) win = stdscr;
    win->_attrs |= A_STANDOUT;
    return OK;
}

int standout(void) {
    return wstandout(stdscr);
}

int wstandend(WINDOW *win) {
    if (!win) win = stdscr;
    win->_attrs &= ~A_STANDOUT;
    return OK;
}

int standend(void) {
    return wstandend(stdscr);
}

int touchwin(WINDOW *win) {
    (void)win;
    // Mark curscr invalid so everything will be refreshed
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            curscr->_lines[y][x] = 0xFFFF;
        }
    }
    return OK;
}

int clearok(WINDOW *win, bool bf) {
    (void)win;
    g_clearok = bf;
    return OK;
}

int idlok(WINDOW *win, bool bf) {
    (void)win; (void)bf;
    return OK;
}

int leaveok(WINDOW *win, bool bf) {
    (void)win; (void)bf;
    return OK;
}

int keypad(WINDOW *win, bool bf) {
    (void)win; (void)bf;
    return OK;
}

int raw(void) { return OK; }
int noraw(void) { return OK; }
int echo(void) { return OK; }
int noecho(void) { return OK; }
int cbreak(void) { return OK; }
int nocbreak(void) { return OK; }

int flushinp(void) {
    while (getchar_timeout_us(0) >= 0) {
        // empty buffer
    }
    return OK;
}

int mvcur(int ly, int lx, int y, int x) {
    (void)ly; (void)lx;
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
    return OK;
}

int wrefresh(WINDOW *win) {
    if (!win) win = stdscr;

    bool force_all = g_clearok;
    g_clearok = false;

    if (force_all) {
        printf("\033[2J");
        for (int r = 0; r < LINES; r++) {
            for (int c = 0; c < COLS; c++) {
                curscr->_lines[r][c] = 0xFFFF;
            }
        }
    }

    bool in_standout = false;
    int last_cursor_y = -1;
    int last_cursor_x = -1;

    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            chtype wc = win->_lines[y][x];
            if (force_all || curscr->_lines[y][x] != wc) {
                if (last_cursor_y != y || last_cursor_x != x) {
                    printf("\033[%d;%dH", y + 1, x + 1);
                    last_cursor_y = y;
                    last_cursor_x = x;
                }

                bool is_so = (wc & A_STANDOUT) != 0;
                if (is_so != in_standout) {
                    in_standout = is_so;
                    if (in_standout) {
                        printf("\033[7m");
                    } else {
                        printf("\033[0m");
                    }
                }

                char c = (char)(wc & A_CHARTEXT);
                if (c < 32 || c > 126) c = ' ';
                putchar(c);

                curscr->_lines[y][x] = wc;
                last_cursor_x++;
            }
        }
    }

    if (in_standout) {
        printf("\033[0m");
    }

    // Move hardware cursor to current position in window
    printf("\033[%d;%dH", win->_cury + 1, win->_curx + 1);
    fflush(stdout);

    return OK;
}

int refresh(void) {
    return wrefresh(stdscr);
}

/*
 * ANSI Escape sequence decoder for arrow keys, navigation, etc.
 */
int getch(void) {
    int c = getchar_timeout_us(1000000); // 1s timeout
    while (c < 0) {
        c = getchar_timeout_us(1000000);
    }

    if (c == 27) { // ESC
        // Check if there is an escape sequence following
        int c2 = getchar_timeout_us(30000); // 30ms timeout
        if (c2 < 0) {
            return 27; // Bare ESC key
        }

        if (c2 == '[') {
            int c3 = getchar_timeout_us(30000);
            if (c3 < 0) return 27;

            // Arrow keys: \033[A, \033[B, \033[C, \033[D
            if (c3 == 'A') return 'k'; // UP -> rogue 'k'
            if (c3 == 'B') return 'j'; // DOWN -> rogue 'j'
            if (c3 == 'C') return 'l'; // RIGHT -> rogue 'l'
            if (c3 == 'D') return 'h'; // LEFT -> rogue 'h'
            if (c3 == 'H') return 'y'; // HOME -> rogue 'y' (up-left)
            if (c3 == 'F') return 'b'; // END -> rogue 'b' (down-left)

            // Extended keys: \033[1~, \033[2~, ...
            if (c3 >= '0' && c3 <= '9') {
                int c4 = getchar_timeout_us(30000);
                if (c4 == '~') {
                    if (c3 == '1' || c3 == '7') return 'y'; // Home -> 'y'
                    if (c3 == '4' || c3 == '8') return 'b'; // End -> 'b'
                    if (c3 == '5') return 'u'; // PgUp -> 'u' (up-right)
                    if (c3 == '6') return 'n'; // PgDn -> 'n' (down-right)
                    if (c3 == '3') return '\b'; // Del
                } else if (c4 == ';') {
                    // Shifted arrows: \033[1;2A etc.
                    int c5 = getchar_timeout_us(30000); // '2'
                    int c6 = getchar_timeout_us(30000); // modifier target
                    if (c5 == '2') {
                        if (c6 == 'A') return 'K';
                        if (c6 == 'B') return 'J';
                        if (c6 == 'C') return 'L';
                        if (c6 == 'D') return 'H';
                    }
                }
            }
            return 27;
        } else if (c2 == 'O') {
            // SS3 keypad / arrows: \033OA, \033OB, etc.
            int c3 = getchar_timeout_us(30000);
            if (c3 == 'A') return 'k';
            if (c3 == 'B') return 'j';
            if (c3 == 'C') return 'l';
            if (c3 == 'D') return 'h';
            if (c3 == 'H') return 'y';
            if (c3 == 'F') return 'b';
            return 27;
        }
        return 27;
    }

    if (c == '\r') {
        return '\n';
    }
    if (c == 127) {
        return '\b';
    }

    return c;
}

int wgetch(WINDOW *win) {
    (void)win;
    return getch();
}

int wgetnstr(WINDOW *win, char *str, int maxlen) {
    if (!win) win = stdscr;
    int idx = 0;
    while (idx < maxlen - 1) {
        int c = getch();
        if (c == '\n' || c == '\r') {
            break;
        }
        if (c == '\b' || c == 127) {
            if (idx > 0) {
                idx--;
                waddch(win, '\b');
                waddch(win, ' ');
                waddch(win, '\b');
                wrefresh(win);
            }
            continue;
        }
        if (c >= 32 && c <= 126) {
            str[idx++] = (char)c;
            waddch(win, (char)c);
            wrefresh(win);
        }
    }
    str[idx] = '\0';
    return OK;
}

char *unctrl(int ch) {
    static char buf[3];
    ch &= 0xff;
    if (ch < 32) {
        buf[0] = '^';
        buf[1] = (char)(ch + '@');
        buf[2] = '\0';
    } else if (ch == 127) {
        buf[0] = '^';
        buf[1] = '?';
        buf[2] = '\0';
    } else {
        buf[0] = (char)ch;
        buf[1] = '\0';
    }
    return buf;
}

int baudrate(void) {
    return 115200;
}

int isendwin(void) {
    return 0;
}

int erasechar(void) {
    return '\b';
}

int killchar(void) {
    return 0x15;
}
