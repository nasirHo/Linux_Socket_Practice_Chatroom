//
// Created by nasirho on 2022/5/31.
//
//
// Created by nasirho on 2022/5/31.
//
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <thread>

using namespace std;

void send_msg(int client_socket);
void recv_msg(int client_socket);
void backSpace(int count);

thread sendThread, recvThread;
unsigned int startIndex = 0;

int main(){
    char name[64];
    cout << "Hello, who are you?\nEnter your name: ";
    cin.getline(name, 64);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
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
        perror("connnet: ");
        exit(-1);
    }



    send(client_socket, name, sizeof(name), 0);
    recv(client_socket, &startIndex, sizeof(unsigned int), 0);
    //cout << "startIndex is " << startIndex << endl;
    thread t1(send_msg, client_socket);
    thread t2(recv_msg, client_socket);

    sendThread = move(t1);
    recvThread = move(t2);

    if(sendThread.joinable())
        sendThread.join();
    if(recvThread.joinable())
        recvThread.join();

    close(client_socket);

    return 0;
}

void send_msg(int client_socket){
    short type = 1;
    while(1){
        cout << "You: ";
        char msg[256];
        cin.getline(msg, 256);
        send(client_socket, &type, sizeof(short),0);
        send(client_socket, msg, sizeof(msg), 0);
    }
}

void recv_msg(int client_socket){
    short type = 0;
    while(1){
        unsigned int index;
        char msg[256];
        int bytes_recv = recv(client_socket, &index,sizeof(unsigned int), 0);
        if(bytes_recv <= 0)
            continue;
        //cout << "receive index: " << index << endl;
        send(client_socket, &type, sizeof(short),0);
        send(client_socket, &startIndex, sizeof(unsigned int), 0);
        for(;startIndex < index; ++startIndex){
            recv(client_socket, msg, sizeof(msg),0);
            backSpace(5);
            cout << msg << endl;
        }
        cout << "You: ";
        fflush(stdout);
    }
}

void backSpace(int count){
    char back_space = 8;
    for(int i = 0; i < count; ++i)
        cout << back_space;
}