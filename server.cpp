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
#include <unistd.h>
#include <mutex>
#include <shared_mutex>

using namespace std;

void stop_server(int signal);
void client_initialize(int client_socket, int id);
void broadcast_index();
void writer(int id, const string& name, const string& msg);
void reader(int client_socket, int id, unsigned int startIndex,unsigned int endIndex);
void endConnection(int id);

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

//int reader_count = 0;
//sem_t resource_mutex;
//sem_t reader_mutex;
mutex clients_mutex;

shared_timed_mutex g_mutex;

int main(){
//    sem_init(&resource_mutex, 0, 1);
//    sem_init(&reader_mutex,0,1);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1){
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in server{};
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
        perror("bind error: ");
        exit(-1);
    }

    if(listen(server_socket, 8) == -1){
        perror("listen error: ");
        exit(-1);
    }

    signal(SIGTERM, stop_server);
    signal(SIGINT, stop_server);
    int client_socket = 0;
    struct sockaddr_in client{};
    unsigned int addrlen = sizeof(client);

    int seed = 0;
    while(true){
        client_socket = accept(server_socket, (struct sockaddr *)&client, &addrlen);
        if(client_socket == -1){
            perror("accept error: ");
            exit(-1);
        }
        seed++;
        thread t(client_initialize, client_socket, seed);
        lock_guard<mutex> guard(clients_mutex);
        clients.push_back({seed, string("Anonymous"), client_socket, (move(t))});
    }

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
    unsigned int startIndex, endIndex;
    recv(client_socket, name, sizeof(name),0);
    for(auto & client : clients){
        if(client.id == id){
            client.name = string(name);
        }
    }
    string welcomeMsg = (string)name + " has entered.";
    writer(0, "System", welcomeMsg);
    while(true){
        long bytes_recv = recv(client_socket, &type, sizeof(short), 0);
        if(bytes_recv <= 0)
            return;

        if(type == 0){
            //cout << "request from " << name << " type 0" << endl;
            recv(client_socket, &startIndex, sizeof(unsigned int), 0);
            recv(client_socket, &endIndex, sizeof(unsigned int), 0);
            reader(client_socket, id, startIndex, endIndex);
        }else if(type == 1){
            recv(client_socket, msg, sizeof(msg), 0);
            writer(id, (string) name,(string)msg);
        }else if(type == 2){
            writer(0, "System", (string) name + " has left.");
            endConnection(id);
        }else{
            cout << "wierd thing";
        }
    }
}

void broadcast_index(){
    unsigned int index = msg_pool.size();
    //cout << "try to boracat index: " << index << endl;
    for(auto & client : clients){
        send(client.socket, &index, sizeof(index), 0);
    }
}

void reader(int client_socket, int id, unsigned int startIndex,unsigned int endIndex){
    char name[64], buff[256];
    int msg_id;

//    sem_wait(&reader_mutex);
//    reader_count++;
//    if (reader_count == 1)
//        sem_wait(&resource_mutex);
//    sem_post(&reader_mutex);

    shared_lock<shared_timed_mutex> r_lock(g_mutex);

    for(unsigned int i = startIndex; i < endIndex; ++i){
        if(msg_pool[i].id == id)
            strcpy(name, "You");
        else
            strcpy(name, msg_pool[i].name.c_str());
        msg_id = msg_pool[i].id;
        strcpy(buff, msg_pool[i].msg.c_str());
        send(client_socket, name, sizeof(name), 0);
        send(client_socket, &msg_id, sizeof(int), 0);
        send(client_socket, buff, sizeof(buff), 0);
    }
//    sem_wait(&reader_mutex);
//    reader_count--;
//    if (reader_count == 0)
//        sem_post(&resource_mutex);
//    sem_post(&reader_mutex);
}

void writer(int id, const string& name, const string& msg){
//    sem_wait(&resource_mutex);
    unique_lock<shared_timed_mutex> w_lock(g_mutex);
    msg_pool.push_back({name, id, msg});
    broadcast_index();
    cout <<msg_pool[msg_pool.size() -1].name << "("<< msg_pool[msg_pool.size() -1].id <<"): " << msg_pool[msg_pool.size() -1].msg<<endl;
//    sem_post(&resource_mutex);
}

void endConnection(int id){
    for(int i = 0; i < clients.size(); ++i){
        if(clients[i].id == id){
            lock_guard<mutex> guard(clients_mutex);
            clients[i].th.detach();
            close(clients[i].socket);
            clients.erase(clients.begin()+i);
            break;
        }
    }
}