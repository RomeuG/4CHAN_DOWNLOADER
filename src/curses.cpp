#include <Client.hpp>
#include <curses.h>

auto curses_catalog(Catalog catalog) -> void
{
    int width;
    int height;

    WINDOW* window = 0;

    auto mainscr = initscr();
    if (!mainscr) {
        return;
    }

    getmaxyx(stdscr, height, width);
    window = newwin(height - 2, width - 2, 1, 1);

    scrollok(window, 1);

    for (;;) {
        wprintw(window, "Window dimensions: %d:%d\n", width, height);
        wrefresh(window);
    }

    endwin();
}
