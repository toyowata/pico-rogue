/*
 * pico_curses.c - Curses implementation over ANSI VT100 USB Serial for RP2040
 * With multi-byte UTF-8 full-width Japanese character support (2 columns)
 */

#include "curses.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static WINDOW _stdscr_inst;
static WINDOW _curscr_inst;

WINDOW *stdscr = &_stdscr_inst;
WINDOW *curscr = &_curscr_inst;

static bool g_clearok = false;

static void set_empty_cell(Cell *cell) {
    cell->ch[0] = ' ';
    cell->ch[1] = '\0';
    cell->width = 1;
    cell->attrs = A_NORMAL;
}

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
            set_empty_cell(&win->_lines[y][x]);
        }
    }
}

/*
 * Return the byte length of the UTF-8 character at s and store its display
 * width (1 or 2 columns) in *char_width.  Malformed or truncated sequences
 * are treated as a single 1-column byte so callers never skip past the NUL.
 */
int utf8_char_info(const char *s, int *char_width) {
    const unsigned char *u = (const unsigned char *)s;
    int blen, width;

    if (u[0] < 0x80) {
        blen = 1;
        width = 1;
    } else if ((u[0] & 0xE0) == 0xC0) {
        blen = 2;
        width = 1;
    } else if ((u[0] & 0xF0) == 0xE0) {
        blen = 3;
        // Half-width Katakana: U+FF61 - U+FF9F (0xEF 0xBD 0xA1 - 0xEF 0xBE 0x9F)
        if (u[0] == 0xEF && ((u[1] == 0xBD && u[2] >= 0xA1) || (u[1] == 0xBE && u[2] <= 0x9F))) {
            width = 1;
        } else {
            // Kanji, Hiragana, Katakana, Full-width symbols
            width = 2;
        }
    } else if ((u[0] & 0xF8) == 0xF0) {
        blen = 4;
        width = 2;
    } else {
        blen = 1;
        width = 1;
    }

    for (int i = 1; i < blen; i++) {
        if ((u[i] & 0xC0) != 0x80) {
            blen = 1;
            width = 1;
            break;
        }
    }

    *char_width = width;
    return blen;
}

/*
 * Return the display width in columns of a UTF-8 string.
 */
int utf8_strwidth(const char *s) {
    int w = 0;
    while (*s) {
        int cw;
        s += utf8_char_info(s, &cw);
        w += cw;
    }
    return w;
}

/*
 * If cell (y, x) is one half of a full-width character, blank the other half
 * so that overwriting it never leaves a stale leading or trail cell behind.
 */
static void split_wide_at(WINDOW *win, int y, int x) {
    Cell *cell = &win->_lines[y][x];
    if (cell->width == 0 && x > 0 && win->_lines[y][x - 1].width == 2) {
        set_empty_cell(&win->_lines[y][x - 1]);
    } else if (cell->width == 2 && x + 1 < COLS && win->_lines[y][x + 1].width == 0) {
        set_empty_cell(&win->_lines[y][x + 1]);
    }
}

WINDOW *initscr(void) {
    init_window_struct(stdscr, LINES, COLS, 0, 0);
    init_window_struct(curscr, LINES, COLS, 0, 0);

    // Invalidate curscr buffer so initial refresh draws everything
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            curscr->_lines[y][x].ch[0] = '\0';
            curscr->_lines[y][x].width = 0xFF;
            curscr->_lines[y][x].attrs = 0xFFFF;
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
    if (win && win != stdscr && win != curscr) {
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
        split_wide_at(win, win->_cury, win->_curx);
        Cell *cell = &win->_lines[win->_cury][win->_curx];
        cell->ch[0] = c;
        cell->ch[1] = '\0';
        cell->width = 1;
        cell->attrs = win->_attrs | (ch & ~A_CHARTEXT);

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

    const char *p = str;
    while (*p) {
        unsigned char c = (unsigned char)*p;
        if (c < 0x80) {
            waddch(win, c);
            p++;
        } else {
            int width;
            int blen = utf8_char_info(p, &width);

            if (win->_curx + width > COLS) {
                win->_curx = 0;
                if (win->_cury < LINES - 1) win->_cury++;
            }

            if (win->_cury < LINES) {
                split_wide_at(win, win->_cury, win->_curx);
                if (width == 2) {
                    split_wide_at(win, win->_cury, win->_curx + 1);
                }

                // Leading cell
                Cell *c1 = &win->_lines[win->_cury][win->_curx];
                memset(c1->ch, 0, sizeof(c1->ch));
                for (int i = 0; i < blen && i < 4; i++) {
                    c1->ch[i] = p[i];
                }
                c1->ch[blen < 4 ? blen : 4] = '\0';
                c1->width = (uint8_t)width;
                c1->attrs = win->_attrs;
                win->_curx++;

                if (width == 2 && win->_curx < COLS) {
                    // Trail cell for full-width character
                    Cell *c2 = &win->_lines[win->_cury][win->_curx];
                    memset(c2->ch, 0, sizeof(c2->ch));
                    c2->width = 0;
                    c2->attrs = win->_attrs;
                    win->_curx++;
                }

                if (win->_curx >= COLS) {
                    win->_curx = 0;
                    if (win->_cury < LINES - 1) win->_cury++;
                }
            }
            p += blen;
        }
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
        return (chtype)(unsigned char)win->_lines[win->_cury][win->_curx].ch[0];
    }
    return ' ';
}

int werase(WINDOW *win) {
    if (!win) win = stdscr;
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            set_empty_cell(&win->_lines[y][x]);
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
        split_wide_at(win, y, win->_curx);
        for (int x = win->_curx; x < COLS; x++) {
            set_empty_cell(&win->_lines[y][x]);
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
            set_empty_cell(&win->_lines[y][x]);
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
    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            curscr->_lines[y][x].ch[0] = '\0';
            curscr->_lines[y][x].width = 0xFF;
            curscr->_lines[y][x].attrs = 0xFFFF;
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
                curscr->_lines[r][c].ch[0] = '\0';
                curscr->_lines[r][c].width = 0xFF;
                curscr->_lines[r][c].attrs = 0xFFFF;
            }
        }
    }

    bool in_standout = false;
    int last_cursor_y = -1;
    int last_cursor_x = -1;

    for (int y = 0; y < LINES; y++) {
        for (int x = 0; x < COLS; x++) {
            Cell *wc = &win->_lines[y][x];
            Cell *cc = &curscr->_lines[y][x];

            bool diff = (wc->attrs != cc->attrs) || (wc->width != cc->width) || (strcmp(wc->ch, cc->ch) != 0);

            if (force_all || diff) {
                // If this is a trail cell (width == 0), cursor has already advanced due to wide char
                if (wc->width == 0) {
                    *cc = *wc;
                    continue;
                }

                if (last_cursor_y != y || last_cursor_x != x) {
                    printf("\033[%d;%dH", y + 1, x + 1);
                    last_cursor_y = y;
                    last_cursor_x = x;
                }

                bool is_so = (wc->attrs & A_STANDOUT) != 0;
                if (is_so != in_standout) {
                    in_standout = is_so;
                    printf(in_standout ? "\033[7m" : "\033[0m");
                }

                if (wc->ch[1] != '\0') {
                    // Multi-byte UTF-8 character (full-width or half-width Katakana etc.)
                    printf("%s", wc->ch);
                } else {
                    unsigned char c = (unsigned char)wc->ch[0];
                    if (c < 32 || c > 126) c = ' ';
                    putchar(c);
                }
                last_cursor_x += wc->width;

                *cc = *wc;
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
        int c2 = getchar_timeout_us(30000); // 30ms timeout
        if (c2 < 0) {
            return 27; // Bare ESC key
        }

        if (c2 == '[') {
            // CSI: parameter bytes (0x30-0x3F) followed by a final byte (0x40-0x7E).
            // Always consume the whole sequence so no stray byte (e.g. '~' of F5)
            // is later interpreted as a rogue command.
            char params[8];
            int plen = 0;
            int fin;
            for (;;) {
                fin = getchar_timeout_us(30000);
                if (fin < 0) return 27;
                if (fin >= 0x30 && fin <= 0x3F) {
                    if (plen < (int)sizeof(params) - 1) params[plen++] = (char)fin;
                    continue;
                }
                if (fin >= 0x20 && fin <= 0x2F) continue; // intermediate bytes
                break;
            }
            params[plen] = '\0';

            if (fin == 'A' || fin == 'B' || fin == 'C' || fin == 'D' || fin == 'H' || fin == 'F') {
                static const char plain[] = "kjlhyb";   // UP DOWN RIGHT LEFT HOME END
                static const char shifted[] = "KJLHYB";
                int idx = (int)(strchr("ABCDHF", fin) - "ABCDHF");
                if (plen == 0 || strcmp(params, "1") == 0) return plain[idx];
                if (strcmp(params, "1;2") == 0) return shifted[idx]; // Shift + key: run
                return 27;
            }
            if (fin == '~') {
                if (strcmp(params, "1") == 0 || strcmp(params, "7") == 0) return 'y'; // Home
                if (strcmp(params, "4") == 0 || strcmp(params, "8") == 0) return 'b'; // End
                if (strcmp(params, "5") == 0) return 'u'; // PgUp -> up-right
                if (strcmp(params, "6") == 0) return 'n'; // PgDn -> down-right
                if (strcmp(params, "3") == 0) return '\b'; // Del
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
