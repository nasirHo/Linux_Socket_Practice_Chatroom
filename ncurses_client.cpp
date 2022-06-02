//
// Created by nasirho on 2022/5/31.
//

#include "ncurses_client.h"
#include <cstring>
#include <cmath>
#include <locale.h>

extern NcursesType ncurses_data;
// 初始化 ncurses 界面
void ncurses_init()
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    if(max_y < 10){
        mvprintw(0,0,"Your terminal is too small!");
        getch();
        exit(1);
    }

    if(has_colors() == FALSE){
        mvprintw(0,0,"Your terminal doesn't support color!");
        getch();
        exit(1);
    }
    start_color();

    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);

    ncurses_data.name_display_height = 3;
    ncurses_data.name_display_width = max_x;
    ncurses_data.name_display_win = newwin(ncurses_data.name_display_height, ncurses_data.name_display_width, 0, 0);
    box(ncurses_data.name_display_win, 0, 0);
    wbkgd(ncurses_data.name_display_win, COLOR_PAIR(1));

    ncurses_data.message_display_height = max_y - 5 - ncurses_data.name_display_height;
    ncurses_data.message_display_width = max_x;
    ncurses_data.message_display_win = newwin(ncurses_data.message_display_height, ncurses_data.message_display_width, ncurses_data.name_display_height, 0);
    scrollok(ncurses_data.message_display_win, TRUE);

    ncurses_data.input_board_height = 5;
    ncurses_data.input_board_width = max_x;
    ncurses_data.input_board_win = newwin(ncurses_data.input_board_height, ncurses_data.input_board_width, ncurses_data.name_display_height + ncurses_data.message_display_height, 0);

    sem_init(&(ncurses_data.message_display_mutex), 0, 1);

    refresh();
}

// 在 消息接收窗口中顯示message信息。
// message放在窗口的最後一行，窗口向上滾動
// 消息接收窗口 的最後一行不顯示字符，和 下面的 消息輸入窗口 分開
void ncurses_message_display(char *message, bool isSelf)
{
    sem_wait(&(ncurses_data.message_display_mutex));
    int scroll_lines = ceil(strlen(message) * 1.0 / ncurses_data.message_display_width);
    wscrl(ncurses_data.message_display_win, scroll_lines);
    if (isSelf)
        wattron(ncurses_data.message_display_win ,COLOR_PAIR(2));
    mvwprintw(ncurses_data.message_display_win,
              ncurses_data.message_display_height - scroll_lines - 1,
              0,
              message);
    if (isSelf)
        wattroff(ncurses_data.message_display_win, COLOR_PAIR(2));
    wrefresh(ncurses_data.message_display_win);
    sem_post(&(ncurses_data.message_display_mutex));
}

// 將window的第y行，第x列後面的字符清空
void ncurses_clear_line(WINDOW *win, int y, int x)
{
    wmove(win, y, x);
    wclrtoeol(win);
}