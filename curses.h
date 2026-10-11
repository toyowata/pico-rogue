/*
 * curses.h - Lightweight Curses-compatible layer for Raspberry Pi Pico
 *
 * Implements the minimal subset of curses needed for Rogue 5.4.
 * Outputs ANSI/VT100 escape sequences over USB Serial (stdio).
 */

#ifndef _CURSES_H_
#define _CURSES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LINES 24
#define COLS  80

#define ERR (-1)
#define OK  0

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef uint16_t chtype;

#define A_NORMAL    0x0000
#define A_STANDOUT  0x0100
#define A_CHARTEXT  0x00ff

/* Window structure representing a 24x80 text grid */
typedef struct _win_st {
    int _cury;
    int _curx;
    int _maxy;
    int _maxx;
    int _begy;
    int _begx;
    int _flags;
    chtype _attrs;
    chtype _lines[LINES][COLS];
} WINDOW;

extern WINDOW *stdscr;
extern WINDOW *curscr;
extern WINDOW *hw;

/* Lifecycle */
WINDOW *initscr(void);
int endwin(void);
WINDOW *newwin(int nlines, int ncols, int begy, int begx);
WINDOW *subwin(WINDOW *orig, int nlines, int ncols, int begy, int begx);
int delwin(WINDOW *win);

/* Cursor and character manipulation */
int wmove(WINDOW *win, int y, int x);
int move(int y, int x);

int waddch(WINDOW *win, chtype ch);
int addch(chtype ch);
int mvaddch(int y, int x, chtype ch);
int mvwaddch(WINDOW *win, int y, int x, chtype ch);

int waddstr(WINDOW *win, const char *str);
int addstr(const char *str);
int mvaddstr(int y, int x, const char *str);

int wprintw(WINDOW *win, const char *fmt, ...);
int printw(const char *fmt, ...);
int mvprintw(int y, int x, const char *fmt, ...);
int mvwprintw(WINDOW *win, int y, int x, const char *fmt, ...);

chtype winch(WINDOW *win);
#define inch() winch(stdscr)
#define mvinch(y, x) (move((y), (x)) == ERR ? ERR : inch())
#define mvwinch(win, y, x) (wmove((win), (y), (x)) == ERR ? ERR : winch(win))

#define getyx(win, y, x) do { (y) = (win)->_cury; (x) = (win)->_curx; } while(0)
#define getmaxy(win) ((win)->_maxy)
#define getmaxx(win) ((win)->_maxx)
#define getbegy(win) ((win)->_begy)
#define getbegx(win) ((win)->_begx)

/* Clearing functions */
int werase(WINDOW *win);
int erase(void);
int wclear(WINDOW *win);
int clear(void);
int wclrtoeol(WINDOW *win);
int clrtoeol(void);
int wclrtobot(WINDOW *win);
int clrtobot(void);

/* Attributes */
int wstandout(WINDOW *win);
int standout(void);
int wstandend(WINDOW *win);
int standend(void);

/* Window options */
int touchwin(WINDOW *win);
int clearok(WINDOW *win, bool bf);
int idlok(WINDOW *win, bool bf);
int leaveok(WINDOW *win, bool bf);
int keypad(WINDOW *win, bool bf);

/* Terminal modes (stubs for Pico) */
int raw(void);
int noraw(void);
int echo(void);
int noecho(void);
int cbreak(void);
int nocbreak(void);

int flushinp(void);
int mvcur(int ly, int lx, int y, int x);

/* Refresh & Output */
int wrefresh(WINDOW *win);
int refresh(void);

/* Input */
int getch(void);
int wgetch(WINDOW *win);
int wgetnstr(WINDOW *win, char *str, int maxlen);

char *unctrl(int ch);
int baudrate(void);
int isendwin(void);
int erasechar(void);
int killchar(void);

#ifdef __cplusplus
}
#endif

#endif /* _CURSES_H_ */
