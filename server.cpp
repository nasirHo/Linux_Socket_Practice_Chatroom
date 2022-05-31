//
// Created by nasirho on 2022/5/31.
//
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <thread>
#include <csignal>

using namespace std;

void stop_server(int server_socket);
void client_initialize(int client_socket, int id);
void broadcast_index();
void writer(int id, const string& msg);
void reader(int client_socket, int startIndex, int endIndex);

struct terminal
{
    int id;
    string name;
    int socket;
    thread th;
};
struct message_pack{
    string name;
    int id;
    string msg;
};
vector<terminal> clients;
vector<message_pack> msg_pool;
int server_socket;

int main(){
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 0);

    if(bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
        perror("bind error: ");
        exit(-1);
    }

    if(listen(server_socket, 8) == -1){
        perror("listen error: ");
        exit(-1);
    }

    signal(SIGTERM, stop_server);
    int client_socket = 0;
    struct sockaddr_in client{};
    unsigned int addrlen = sizeof(client);

    char msg[] = "This is server";
    char buffer[256] = {};

    int seed = 0;
    while(1){
        client_socket = accept(server_socket, (struct sockaddr *)&client, &addrlen);
        if(client_socket == -1){
            perror("accept error: ");
            exit(-1);
        }
        seed++;
        thread t(client_initialize, client_socket, seed);
        clients.push_back({seed, string("Anonymous"), client_socket, (move(t))});
    }

    for(auto & client : clients){
        if(client.th.joinable())
            client.th.join();
    }

    close(server_socket);
    return 0;
}

void stop_server(int signal){
    for(auto & client : clients){
        if(client.th.joinable())
            client.th.join();
    }

    close(server_socket);
    exit(signal);
}

void client_initialize(int client_socket, int id){
    char name[64], msg[256];
    short type;
    unsigned int startIndex;
    recv(client_socket, name, sizeof(name),0);
    for(auto & client : clients){
        if(client.id == id){
            client.name = string(name);
        }
    }
    string welcomMsg = (string)name + " has entered.";
    msg_pool.push_back({(string)name, id, welcomMsg});
    cout <<msg_pool[msg_pool.size() -1].name << ": " << msg_pool[msg_pool.size() -1].msg<<endl;
    broadcast_index();
    while(1){
        int bytes_recv = recv(client_socket, &type, sizeof(short), 0);
        if(bytes_recv <= 0)
            return;

        if(type == 0){
            cout << "request from " << name << " type 0" << endl;
            recv(client_socket, &startIndex, sizeof(unsigned int), 0);
            reader(client_socket, startIndex, msg_pool.size());
        }else if(type == 1){
            recv(client_socket, msg, sizeof(msg), 0);
            writer(id, (string)msg);
            cout <<msg_pool[msg_pool.size() -1].name << ": " << msg_pool[msg_pool.size() -1].msg<<endl;
        }else{
            cout << "wierd thing";
        }
    }



}

void broadcast_index(){
    unsigned int index = msg_pool.size();
    cout << "try to boracat index: " << index << endl;
    for(auto & client : clients){
        send(client.socket, &index, sizeof(index), 0);
    }
}

void reader(int client_socket, int startIndex, int endIndex){
    char buff[256];
    for(int i = startIndex; i < endIndex; ++i){
        strcpy(buff, msg_pool[i].msg.c_str());
        send(client_socket, buff, sizeof(buff), 0);
    }
}

void writer(int id, const string& msg){
    msg_pool.push_back({"unknown", id, msg});
    broadcast_index();
}