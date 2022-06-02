//
// Created by nasirho on 2022/5/31.
//
#include <ncurses.h>
#include <string>
#include <cstring>
#include <csignal>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "ncurses_client.h"
using namespace std;

void send_msg();
void recv_msg();
void stop_client(int signal);

thread sendThread, recvThread;
unsigned int startIndex = 0;
int client_socket;
NcursesType ncurses_data;
bool exitFlag = false;

int main(int argc, char **argv)
{
    ncurses_init();
    WINDOW *input_board_win = ncurses_data.input_board_win;

    char name[64], welcom_msg[72];

    mvwprintw(ncurses_data.name_display_win, 1, 2, "Hello, who are you?");
    wrefresh(ncurses_data.name_display_win);
    mvwprintw(input_board_win, 2, 0, "Enter Name: ");
    wgetstr(input_board_win, name);
    ncurses_clear_line(ncurses_data.name_display_win, 1, 2);
    ncurses_clear_line(input_board_win, 2, 0);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == -1){
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in client{};
    client.sin_family = AF_INET;
    client.sin_port = htons(8080);
    client.sin_addr.s_addr = INADDR_ANY;
    bzero(&client.sin_zero, 0);

    if(connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1){
        perror("connect: ");
        exit(-1);
    }

    signal(SIGTERM, stop_client);
    signal(SIGINT, stop_client);
    signal(SIGWINCH, stop_client);

    sprintf(welcom_msg, "Welcome, %s", name);
    //wattron(ncurses_data.name_display_win ,COLOR_PAIR(1));
    mvwprintw(ncurses_data.name_display_win, 1, 2, welcom_msg);
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
    return 0;
}


void send_msg(){
    short type = 1;
    char message[256];
    while(true){
        mvwprintw(ncurses_data.input_board_win, 2, 0, "message: ");
        wgetstr(ncurses_data.input_board_win, message);
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

void recv_msg(){
    short type = 0;
    while(true){
        if(exitFlag) return;
        unsigned int index;
        int id;
        char name[64], msg[256], display_buff[512];
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
            ncurses_message_display(display_buff, strcmp(name, "You") == 0, id == 0);
            wrefresh(ncurses_data.input_board_win);
        }
    }
}


void stop_client(int signal){
    short type = 2;
    send(client_socket, &type, sizeof(short), 0);
    exitFlag = true;
    sendThread.detach();
    recvThread.detach();
    close(client_socket);
    exit(signal);
}