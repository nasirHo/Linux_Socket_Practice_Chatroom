//
// Created by nasirho on 2022/5/31.
//

#ifndef CHATROOM_NCURSES_CLIENT_H
#define CHATROOM_NCURSES_CLIENT_H

#include <ncurses.h>
#include <semaphore.h>

typedef struct {
    WINDOW* name_display_win;
    int name_display_height;
    int name_display_width;
    WINDOW* message_display_win;
    int message_display_height;
    int message_display_width;
    WINDOW* input_board_win;
    int input_board_height;
    int input_board_width;

    sem_t message_display_mutex;
} NcursesType;
void ncurses_init();
void ncurses_message_display(char* message, short color, bool isSystem);
void ncurses_clear_line(WINDOW* win, int y, int x);


#endif //CHATROOM_NCURSES_CLIENT_H
