#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <functional>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

namespace Network {
    class Server{
        public:
        explicit Server(int port);
        ~Server();
        json AcceptReq();
        int SendResponse(json response);
        
        private:
        int port;
        int serverSockFd;
        struct sockaddr_in serverAddr;
        struct sockaddr_in clientAddr;
        socklen_t clientLen;
        int clientSockFd;

    };

    class Client {
        public:
        explicit Client(std::string address,int port);
        ~Client();
        json SendData(json data);

        private:
        int sockFd;
        struct sockaddr_in serverAddr;
    };

};