#include <Client.hpp>
#include <curses.h>

auto curses_catalog(Catalog catalog) -> void
{
    int winx;
    int winy;

    WINDOW* window = 0;

    auto mainscr = initscr();
    if (!mainscr) {
        return;
    }

    noecho();
    cbreak();
    keypad(stdscr, 1);
    nodelay(stdscr, 1);

    getmaxyx(stdscr, winy, winx);
    window = newwin(winy - 2, winx - 2, 1, 1);

    scrollok(window, 1);

    // set cursor position
    int curx = window->_curx;
    int cury = window->_cury;

    // main loop
    for (;;) {
        //wprintw(window, "Window dimensions: %d:%d\n", winx, winy);
        int ch;
        if ((ch = getch()) != ERR) {
            if (ch == KEY_DOWN) {
                //wprintw(window, "KEY_DOWN = %d\n");
                wmove(window, ++cury, curx);
            }
        }

        wrefresh(window);
    }

    endwin();
}
