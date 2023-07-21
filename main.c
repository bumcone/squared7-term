/* vim:set ts=4 sw=4 tw=80 et ai si cindent cino=L0,b1,(1s,U1,m1,j1,J1,)50,*90 cinkeys=0{,0},0),0],\:,0#,!^F,o,O,e,0=break:
 */
/**********************************************************************
    7 Curses
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
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <panel.h>

#define SQR_WIDTH  5 //3
#define SQR_HEIGHT 3 //3

WINDOW *BOARD[7*7];
WINDOW *NEW[7];

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

void board_create(void) {
    int i;
    int startx, starty;

    for (starty = 0; starty < 7; ++starty) {
        for (startx = 0; startx < 7; ++startx) {
fprintf(stderr, "Creating BOARD[%d,%d:%d] @ (%d, %d)\n", startx, starty, (starty * 7) + startx, SQR_WIDTH * startx, SQR_HEIGHT * starty);
            BOARD[(starty * 7) + startx] = newwin(
                 SQR_HEIGHT
                ,SQR_WIDTH
                ,SQR_HEIGHT * starty
                ,SQR_WIDTH  * startx
            );
        }
    }

    for (i = 0; i < 7*7; ++i) {
        draw_square(i);
        //wrefresh(BOARD[i]);
    }
}

WINDOW *w_main;
PANEL *p_main;
int label_colour = 1;
int main(int argc, char* argv[]) {
    int key;
    int x, y, w, h;

	initscr();
    if (has_colors()) start_color();

    noecho();
    cbreak();

    clear();
    refresh();

    init_pair(1, COLOR_RED, COLOR_BLACK);

//    mvprintw(LINES-1, (COLS - 5) / 2, "TESTy");

    w_main = newwin(LINES, COLS, 0, 0);

getbegyx(w_main, y, x);
getmaxyx(w_main, h, w);

    box(w_main, 0, 0);
// Title bar
mvwaddch(w_main, 2, 0, ACS_LTEE);
mvwhline(w_main, 2, 1, ACS_HLINE, w - 2);
mvwaddch(w_main, 2, w - 1, ACS_RTEE);
    wrefresh(w_main);

print_middle(w_main, 1, 0, w, "Squared 7", COLOR_PAIR(label_colour));
p_main = new_panel(w_main);
update_panels();

    board_create();

    do {
        key = getch();

        switch (key) {
            case ' ':
            break;
        }
    } while ((key != 'q') && (key != 'Q'));

    endwin();
    return 0;
}


