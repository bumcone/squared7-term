/* vim:set ts=4 sw=4 tw=80 et ai si cindent cino=L0,b1,(1s,U1,m1,j1,J1,)50,*90 cinkeys=0{,0},0),0],\:,0#,!^F,o,O,e,0=break:
 */
/**********************************************************************
    Squared 7 (curses)
    Copyright (C) 2023 Krayon

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 ONLY, as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program, in the file COPYING or COPYING.txt; if
    not, see http://www.gnu.org/licenses/ , or write to:
      The Free Software Foundation, Inc.,
      51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **********************************************************************/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <ncurses.h>
#include <panel.h>

#define SIGEXIT 0

#define SQR_WIDTH  5 //3
#define SQR_HEIGHT 3 //3

#define COLOR_GREY 8//15

#define TIMESTEP 2000

WINDOW *w_scr  = NULL;
WINDOW *w_main = NULL;
PANEL *p_main  = NULL;

int w_scr_w = -1;
int w_scr_h = -1;

int counter=0;

WINDOW *BOARD[7*7];
WINDOW *NEW[7];

int col_label = 1;
int col_sqr_empty = 2;
int oldcur = ERR;

int int_width(int number) {
    if (number == 0) return 1;

    return floor(log10(abs(number))) + 1 + (number < 0 ? 1 : 0);
}

void draw_square(int sq) {
    switch (sq) {
        case 7:
            box(BOARD[sq], '7', '7');
        break;

        case 14:
            wborder(BOARD[sq], '#', '#', '#', '#', '#', '#', '#', '#');
        break;

        case 25:
            box(BOARD[sq], 'X', 'x');
        break;

        case 26:
            box(BOARD[sq], 'O', 'o');
        break;

        default:
            box(BOARD[sq], 0, 0);
    }

    wrefresh(BOARD[sq]);
}

int print_middle(WINDOW *win, int starty, int startx, int width, char*string, chtype color) {
    int len, x, y;
    float tmp;

    if (win == NULL) win = stdscr;

    getyx(win, y, x);
    if (startx != 0) x = startx;
    if (starty != 0) y = starty;
    if (width == 0) width = 80;

    len = strlen(string);
    tmp = (width - len) / 2;
    x = startx + (int)tmp;
    wattron(win, color);
    mvwprintw(win, y, x, "%s", string);
    wattroff(win, color);
    refresh();
}

void board_create(int offy, int offx) {
    int i;
    int startx, starty;

    for (starty = 0; starty < 7; ++starty) {
        for (startx = 0; startx < 7; ++startx) {
fprintf(stderr, "Creating BOARD[%d,%d:%d] @ (%d, %d)\n", startx, starty, (starty * 7) + startx, SQR_WIDTH * startx, SQR_HEIGHT * starty);
            BOARD[(starty * 7) + startx] = newwin(
                 SQR_HEIGHT
                ,SQR_WIDTH
                ,offy + (SQR_HEIGHT * starty)
                ,offx + (SQR_WIDTH  * startx)
            );
        }
    }

    for (i = 0; i < 7*7; ++i) {
        wattron(BOARD[i], COLOR_PAIR(col_sqr_empty));
        draw_square(i);
        wattroff(BOARD[i], COLOR_PAIR(col_sqr_empty));
        //wrefresh(BOARD[i]);
    }
}

// Timer
void init_timer(void) {
    struct itimerval it;

    //  Clear itimerval struct members
    timerclear(&it.it_interval);
    timerclear(&it.it_value);

    // Configure (and start) timer
    it.it_interval.tv_usec = TIMESTEP;
    it.it_value.tv_usec    = TIMESTEP;
    setitimer(ITIMER_REAL, &it, NULL);
}

void cb_timer(int signal) {
    extern WINDOW * w_scr;
    extern int w_scr_w, w_scr_h;

    assert(signal == SIGALRM);

    ++counter;
    //mvwprintw(w_scr, w_scr_h - 1, w_scr_w - int_width(counter) - 2, "%d", counter);
    return;
}

void cb_sighandler(int signal) {
    extern WINDOW * w_main;
    extern int oldcur;

    fprintf(stderr, "SIG: %d\n", signal);

    switch (signal) {
        case SIGEXIT:
        case SIGTERM:
        case SIGQUIT:
        case SIGHUP:
        case SIGINT:
            // Clean up
            fprintf(stderr, "Clean up...\n");
            nocbreak();
            echo();
            clear();
            if (p_main) del_panel(p_main); p_main = NULL;
            if (w_main) delwin(w_main); w_main = NULL;
            if (oldcur != ERR) curs_set(oldcur);
            endwin();
            refresh();
            //free_other_resources();
            exit(EXIT_SUCCESS);
    }
}

void init_signals(void) {
    struct sigaction sa;

    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);

    // Handle SIGINT, SIGHUP, SIGQUIT and SIGTERM via cb_sighandler
    sa.sa_handler = cb_sighandler;
    //sigaction(SIGEXIT,  &sa, NULL);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGHUP,  &sa, NULL);
    sigaction(SIGQUIT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Handle SIGALRM (timer) via cb_timer
    sa.sa_handler = cb_timer;
    sigaction(SIGALRM, &sa, NULL);

    // Ignore SIGTSTP
    sa.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sa, NULL);
}

int main(int argc, char* argv[]) {
    int key;
    int x, y;

    extern int w_scr_w;
    extern int w_scr_h;

    // Seed RNG
    srand((unsigned)time(NULL));

    w_scr = initscr();
    if (has_colors()) start_color();

    init_signals();
    init_timer();

    // Hide cursor
    oldcur = curs_set(0);

    // No local echo
    noecho();

    // Disable line buffering?
    cbreak();

    clear();
    refresh();

    init_color(COLOR_GREY, 100, 100, 100);

    init_pair(col_label, COLOR_RED, COLOR_BLACK);
    init_pair(col_sqr_empty, COLOR_GREY, COLOR_BLACK);

    getbegyx(w_scr, y, x);
    getmaxyx(w_scr, w_scr_h, w_scr_w);

    // Screen window
    wattron(w_scr, COLOR_PAIR(col_label));
    box(w_scr, 0, 0);

    // Title bar border
    mvwaddch(w_scr, 2, 0, ACS_LTEE);
    mvwhline(w_scr, 2, 1, ACS_HLINE, w_scr_w - 2);
    mvwaddch(w_scr, 2, w_scr_w - 1, ACS_RTEE);
    wattroff(w_scr, COLOR_PAIR(col_label));

    // Title bar
    move(1, 1);
    wattron(w_scr, COLOR_PAIR(col_label));
    mvwhline(w_scr, 1, 1, ACS_CKBOARD, w_scr_w - 2);
    wattroff(w_scr, COLOR_PAIR(col_label));
    print_middle(w_scr, 1, 0, w_scr_w, " Squared 7 ", COLOR_PAIR(col_label));

    wrefresh(w_scr);

    // Main window
    w_main = newwin(SQR_HEIGHT * 7, SQR_WIDTH * 7, 3, 1);
    wrefresh(w_main);

    p_main = new_panel(w_main);
    update_panels();

    // Create payfield
    board_create(3, 1);

    int last_counter = 0;
int cur_sqr = 0;
    do {
        key = getch();

        switch (key) {
            case ' ':

wattron(BOARD[cur_sqr], COLOR_PAIR(col_sqr_empty));
draw_square(cur_sqr);
wattroff(BOARD[cur_sqr], COLOR_PAIR(col_sqr_empty));
wrefresh(BOARD[cur_sqr]);

++cur_sqr; if (cur_sqr >= 7 * 7) cur_sqr = 0;
//wbkgd(BOARD[cur_sqr], COLOR_PAIR(col_label));
//wattron(BOARD[cur_sqr], A_BOLD | COLOR_PAIR(col_sqr_empty));
wattron(BOARD[cur_sqr], COLOR_PAIR(col_label));
draw_square(cur_sqr);
wattroff(BOARD[cur_sqr], COLOR_PAIR(col_label));
wrefresh(BOARD[cur_sqr]);

            break;
        }

        if (last_counter < counter) {
            //mvwprintw(w_scr, w_scr_h - 1, w_scr_w - int_width(counter) - 2, "%d", counter);
            //mvwprintw(w_scr, w_scr_h - 1, w_scr_w - int_width(counter) - 2 - int_width(counter - last_counter) - 3, "%d (%d)", counter, counter - last_counter);
            mvwprintw(w_scr, w_scr_h - 1, w_scr_w - 8 - 2 - 4 - 3, "%8d (%4d)", counter, counter - last_counter);
            wrefresh(w_scr);
            last_counter = counter;
        }

    } while ((key != 'q') && (key != 'Q'));

    cb_sighandler(SIGEXIT);

    return 0;
}

