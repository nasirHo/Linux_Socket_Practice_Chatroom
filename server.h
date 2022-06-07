//
// Created by nasirho on 2022/6/3.
//

#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H

#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include "global.h"

struct terminal
{
    int id;
    std::string name;
    int socket;
    std::thread th;
};
struct message_pack{
    std::string name;
    int id;
    std::string msg;
};

class server{
public:
    server();
private:
    static std::vector<terminal> clients;
    static std::vector<message_pack> msg_pool;
    static int server_socket;

    static std::mutex clients_mutex;
    static std::shared_timed_mutex g_mutex;

    static void stop_server(int signal);
    static void client_initialize(int client_socket, int id);
    static inline void broadcast_index();
    static void writer(int id, const std::string& name, const std::string& msg);
    static void reader(int client_socket, int id, unsigned int startIndex,unsigned int endIndex);
    static void endConnection(int id);

};
#endif //CHATROOM_SERVER_H
