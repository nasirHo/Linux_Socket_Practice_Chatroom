# Linux_Socket_Practice_Chatroom
linux socket programming practice -- a more to more chatroom implement by cpp
## Build
### Dependency
- Ubuntu 18.04
  - build-essential
  - cmake (>3.8)
  - libncurses-dev
- Arch
  - base-devel
  - cmake
  - ncurses
### Compile
1. cd to this directory
2. generate cmake configuration
```sh
cmake .
```
3. build
```sh
cmake --build .
```
4. You can find executable in bin directory
## Usage
1. Start the server
```sh
./chatroom_server
```
2. Start the client
```sh
./chatroom_client_ncures
```
3. Enter your name
![](enter.png)
4. Start Chatting
![](chat.png)
5. type /quit to leave
## Limitation
~~- Chinese word cannot display~~
- Clients count has limit
- UI might wrong during resize terminal
