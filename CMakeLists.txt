cmake_minimum_required(VERSION 3.23)
project(chatroom)

set(CMAKE_CXX_STANDARD 14)

add_executable(chatroom_server server.cpp)
add_executable(chatroom_client1 client.cpp)
add_executable(chatroom_client2 client.cpp)
add_executable(chatroom_client3 client.cpp)