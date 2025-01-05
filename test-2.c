#include <stdio.h>
#include <stdlib.h>
#include "pdcurses/curses.h"

void print_menu(WINDOW *menu_win, int highlight, int checked[], char *choices[], int n_choices) {
    int x, y, i;

    x = 2;
    y = 2;

    box(menu_win, 0, 0);
    for (i = 0; i < n_choices; ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
        }
        mvwprintw(menu_win, y, x, "[%c] %s", checked[i] ? 'P' : ' ', choices[i]);
        wattroff(menu_win, A_REVERSE);
        y++;
    }
    wrefresh(menu_win);
}

int main() {
    WINDOW *menu_win;
    int highlight = 1;
    int choice = 0;
    int c;

    char *choices[] = {
        "Option 1",
        "Option 2",
        "Option 3",
        "Option 4",
        "Exit"
    };
    int n_choices = sizeof(choices) / sizeof(char *);
    int checked[n_choices];
    for (int i = 0; i < n_choices; ++i) checked[i] = 0;

    initscr();
    clear();
    noecho();
    cbreak();
    menu_win = newwin(10, 30, 4, 4);
    keypad(menu_win, TRUE);

    print_menu(menu_win, highlight, checked, choices, n_choices);
    while (1) {
        c = wgetch(menu_win);
        switch (c) {
            case KEY_UP:
                if (highlight == 1)
                    highlight = n_choices;
                else
                    --highlight;
                break;
            case KEY_DOWN:
                if (highlight == n_choices)
                    highlight = 1;
                else
                    ++highlight;
                break;
            case ' ':
                checked[highlight - 1] = !checked[highlight - 1];
                break;
            case 10:
                choice = highlight;
                break;
            default:
                break;
        }
        print_menu(menu_win, highlight, checked, choices, n_choices);
        if (choice == n_choices) // Exit selected
            break;
    }

    endwin();
    return 0;
}
