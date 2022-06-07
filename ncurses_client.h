//
// Created by nasirho on 2022/5/31.
//

#ifndef CHATROOM_NCURSES_CLIENT_H
#define CHATROOM_NCURSES_CLIENT_H

#include <ncurses.h>
#include <semaphore.h>
#include <thread>
#include <ncurses.h>
#include <string>
#include <cstring>
#include <csignal>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "global.h"

struct NcursesType{
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
};

class client{
public:
    client();
private:
    static NcursesType ncurses_data;
    static std::thread sendThread, recvThread;
    static unsigned int startIndex;
    static int client_socket;
    static bool exitFlag;

    static void ncurses_init();
    static void ncurses_message_display(char* message, short color, bool isSystem);
    static void ncurses_clear_line(WINDOW* win, int y, int x);
    static void send_msg();
    static void recv_msg();
    static void stop_client(int signal);
};


#endif //CHATROOM_NCURSES_CLIENT_H
