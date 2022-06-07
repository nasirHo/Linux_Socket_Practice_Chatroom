//
// Created by nasirho on 2022/5/31.
//

#include "ncurses_client.h"
#include <cstring>
#include <cmath>
#include <clocale>

using namespace std;

NcursesType client::ncurses_data;
thread client::sendThread, client::recvThread;
unsigned int client::startIndex = 0;
int client::client_socket = 0;
bool client::exitFlag = false;

client::client() {
    ncurses_init();

    char name[NAME_LENGTH_LIMIT], welcomeMsg[NAME_LENGTH_LIMIT + 10];

    mvwprintw(ncurses_data.name_display_win, 1, 2, "Hello, who are you?");
    wrefresh(ncurses_data.name_display_win);
    mvwprintw(ncurses_data.input_board_win, 2, 0, "Enter Name: ");
    wgetstr(ncurses_data.input_board_win, name);
    ncurses_clear_line(ncurses_data.name_display_win, 1, 2);
    ncurses_clear_line(ncurses_data.input_board_win, 2, 0);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1){
        perror("socket: ");
        exit(-1);
    }

    sockaddr_in client{};
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(8080);
    client.sin_addr.s_addr = INADDR_ANY;

    if(connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1){
        perror("connect: ");
        exit(-1);
    }

    signal(SIGTERM, stop_client);
    signal(SIGINT, stop_client);
    signal(SIGWINCH, stop_client);

    sprintf(welcomeMsg, "Welcome, %s", name);
    //wattron(ncurses_data.name_display_win ,COLOR_PAIR(1));
    mvwprintw(ncurses_data.name_display_win, 1, 2, welcomeMsg);
    //wattroff(ncurses_data.name_display_win ,COLOR_PAIR(1));
    wrefresh(ncurses_data.name_display_win);


    send(client_socket, name, sizeof(name), 0);
    recv(client_socket, &startIndex, sizeof(unsigned int), 0);
    thread t1(send_msg);
    thread t2(recv_msg);

    sendThread = move(t1);
    recvThread = move(t2);

    if(sendThread.joinable())
        sendThread.join();
    if(recvThread.joinable())
        recvThread.join();

    close(client_socket);
    endwin();
}
// 初始化 ncurses 界面
void client::ncurses_init()
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

    init_pair(1, COLOR_BLACK, COLOR_WHITE);//name windows color
    init_pair(2, COLOR_MAGENTA, COLOR_YELLOW);//system message color
    init_pair(3, COLOR_BLUE, COLOR_BLACK);//self color
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_WHITE, COLOR_BLACK);
    init_pair(6, COLOR_RED, COLOR_BLACK);
    init_pair(7, COLOR_YELLOW, COLOR_BLACK);
    init_pair(8, COLOR_CYAN, COLOR_BLACK);

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
void client::ncurses_message_display(char *message, short colors, bool isSystem)
{
    sem_wait(&(ncurses_data.message_display_mutex));
    int scroll_lines = ceil(strlen(message) * 1.0 / ncurses_data.message_display_width);
    wscrl(ncurses_data.message_display_win, scroll_lines);

    if (isSystem)
        wattron(ncurses_data.message_display_win ,COLOR_PAIR(2));
    else
        wattron(ncurses_data.message_display_win ,COLOR_PAIR(colors + 3));

    mvwprintw(ncurses_data.message_display_win,
              ncurses_data.message_display_height - scroll_lines - 1,
              0,
              message);

    if (isSystem)
        wattroff(ncurses_data.message_display_win, COLOR_PAIR(2));
    else
        wattroff(ncurses_data.message_display_win, COLOR_PAIR(colors + 3));

    wrefresh(ncurses_data.message_display_win);
    sem_post(&(ncurses_data.message_display_mutex));
}

// 將window的第y行，第x列後面的字符清空
void client::ncurses_clear_line(WINDOW *win, int y, int x)
{
    wmove(win, y, x);
    wclrtoeol(win);
}

void client::send_msg(){
    short type = 1;
    char message[MSG_LENGTH_LIMIT];
    while(true){
        mvwprintw(ncurses_data.input_board_win, 2, 0, "message: ");
        wgetstr(ncurses_data.input_board_win, message);
        if(strlen(message) == 0)
            continue;
        if (strcmp(message, "/quit") == 0) {
            type = 2;
            send(client_socket, &type, sizeof(short), 0);
            exitFlag = true;
            recvThread.detach();
            close(client_socket);
            return;
        }
        send(client_socket, &type, sizeof(short),0);
        send(client_socket, message, sizeof(message), 0);
        ncurses_clear_line(ncurses_data.input_board_win, 2, 9);
    }
}

void client::recv_msg(){
    short type = 0;
    while(true){
        if(exitFlag) return;
        unsigned int index;
        int id;
        char name[NAME_LENGTH_LIMIT], msg[MSG_LENGTH_LIMIT], display_buff[NAME_LENGTH_LIMIT + MSG_LENGTH_LIMIT + 10];
        long bytes_recv = recv(client_socket, &index,sizeof(unsigned int), 0);
        if(bytes_recv <= 0)
            continue;
        //cout << "receive index: " << index << endl;
        if (startIndex >= index) continue;
        send(client_socket, &type, sizeof(short),0);
        send(client_socket, &startIndex, sizeof(unsigned int), 0);
        send(client_socket, &index, sizeof(unsigned int), 0);
        for(;startIndex < index; ++startIndex){
            recv(client_socket, name, sizeof(name),0);
            recv(client_socket, &id, sizeof(int), 0);
            recv(client_socket, msg, sizeof(msg),0);
            sprintf(display_buff, "%s(%d): %s", name, id, msg);
            ncurses_message_display(display_buff, (strcmp(name, "You") != 0) * (id % 5 + 1), id == 0);
            wrefresh(ncurses_data.input_board_win);
        }
    }
}


void client::stop_client(int signal){
    short type = 2;
    send(client_socket, &type, sizeof(short), 0);
    exitFlag = true;
    sendThread.detach();
    recvThread.detach();
    close(client_socket);
    exit(signal);
}

int main(int argc, char **argv){
    client c;

    return 0;
}