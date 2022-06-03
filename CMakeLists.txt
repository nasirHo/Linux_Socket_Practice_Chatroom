cmake_minimum_required(VERSION 3.8)
project(chatroom)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

add_executable(chatroom_server server.cpp)
target_link_libraries(chatroom_server PRIVATE Threads::Threads)


add_executable(chatroom_client_ncurses ncurses_client_main.cpp ncurses_client.cpp ncurses_client.h)
target_link_libraries(chatroom_client_ncurses PRIVATE ${CURSES_LIBRARIES} Threads::Threads)

