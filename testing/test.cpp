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
#include <queue>
#include <algorithm>

using json = nlohmann::json;

std::vector<json> q;

class Server{
    public:
 
    Server(int port) : port(port){
        // int serverSockFd = socket(AF_INET, SOCK_STREAM, 0);
        serverSockFd = socket(AF_INET, SOCK_STREAM, 0);
        if(serverSockFd < 0){
            std::cerr<<"Error creating socket" <<std::endl;
            return;
        }

        // struct sockaddr_in serverAddr;
        std::memset(&serverAddr,0,sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if(bind(serverSockFd,(struct sockaddr*)&serverAddr,sizeof(serverAddr)) < 0) {
            std::cerr<<"Error binding socket" <<std::endl;
            close(serverSockFd);
            return;
        }

        if (listen(serverSockFd, 10) < 0) {
            std::cerr << "Error listening on socket"<<std::endl;
            close(serverSockFd);
            return;
        }

        std::cout<< "Server listening on port " <<port<<std::endl;
        // struct sockaddr_in clientAddr;
        // socklen_t clientLen = sizeof(clientAddr);
        clientLen = sizeof(clientAddr);
        // clientFd = accept(serverSockFd,(struct sockaddr*)&clientAddr,&clientLen);
        
    }

    json AcceptReq(){
        // int clientFd = accept(serverSockFd,(struct sockaddr*)&clientAddr,&clientLen);
       
        clientSockFd = accept(serverSockFd,(struct sockaddr*)&clientAddr,&clientLen);
        
        
        if (clientSockFd < 0) {
            std::cerr << "Error accepting connection\n";
            close(serverSockFd);
            return json{{"error accepting connection",1}};
        }

        std::cout << "Client connected\n";

        char buffer[1024] = {0}; //should add vectors if message is larger than 1024
        int bytesReceived = recv(clientSockFd, buffer, sizeof(buffer), 0);
        std::cout<<bytesReceived<<std::endl;
        if (bytesReceived > 0) {
            std::cout << "Received: " << buffer << std::endl;
            // Sending response back to the client
            // close(clientSockFd);
            return json::parse(std::string(buffer,bytesReceived));
        }
        
        // close(serveSockFd);
        // close(clientSockFd);
        return json{{"Error in accept request",1}};
       
    }

    int SendResponse(json response){
        // const char* response = "Message received!";
        send(clientSockFd, response.dump().c_str(), strlen(response.dump().c_str()), 0);
        close(clientSockFd);
        return 1;
    }


    ~Server(){
        if(clientSockFd > 0) close(clientSockFd);
        if(serverSockFd > 0) close(serverSockFd);
    }

    private:
    int port;
    int serverSockFd;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    socklen_t clientLen;
    int clientSockFd;
    

    
};

class Client{
    public:
    Client(std::string address,int port){
        //int sockFd
        sockFd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockFd < 0) {
            std::cerr << "Error creating socket\n";
            return;
        }

        // struct sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        // Connect to the server at localhost (127.0.0.1)
        if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address\n";
            close(sockFd);
            return;
        }

        if (connect(sockFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Connection failed\n";
            close(sockFd);
            return;
        }

        std::cout << "Connected to server\n";
        // const char* message = "Hello, server!";
        
        // send(sockfd, message, strlen(message), 0);
    }
    json SendData(json data){
        send(sockFd, data.dump().c_str(), strlen(data.dump().c_str()), 0);

        char buffer[1024] = {0};
        int bytesReceived = recv(sockFd, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Server replied: " << buffer << std::endl;
            close(sockFd);
            return json::parse(std::string(buffer,bytesReceived));

        }

        close(sockFd);
        return json({"Error in send data",1});

    }

    ~Client(){
        if(sockFd > 0) close(sockFd);
    }

    private:
    int sockFd;
    struct sockaddr_in serverAddr;


};


// int var = 0;

// void run(){
//     Server server(8080);

//     while(!var){
//         std::cout<<server.AcceptReq()<<std::endl;
//         server.SendResponse("hello");
//     }

// }


// int main() {
    
    // std::unique_ptr<std::thread> s = std::make_unique<std::thread>(start_server);

    // std::this_thread::sleep_for(std::chrono::seconds(30));
    // json obj = {"hello",1};
    // // start_client(obj);
    // s->join();

        // std::thread t(run);

        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // Client c("127.0.0.1",8080);
        // Client d("127.0.0.1",8080);
        // Client e("127.0.0.1",8080);
        // // var=1;
        // c.SendData("hello there from client");
        
        // d.SendData("d sent");

        // var =1;
        // e.SendData("e sent");
        
        // // d.SendData("hello from d");

        // t.join();

        // return 0;
// }

int var=0;

void runServer(){
    Server server(8080);
    while(!var){
        json data = server.AcceptReq();
        if(data["hello from client"] == 1)
            q.push_back(data);

        server.SendResponse(json{{"hi from server", 1}});
    }
}


int main(){

    std::thread t(runServer);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Client a("127.0.0.1",8080);
    Client b("127.0.0.1",8080);
    Client c("127.0.0.1",8080);


   
    std::this_thread::sleep_for(std::chrono::seconds(3));
    a.SendData(json{{"hello from client",1}});
    b.SendData(json{{"hello from client",1}});
    var=1;
    c.SendData(json{{"hello from client c",1}});
    t.join();

    std::cout<<"Printing the queue"<<std::endl;
    for(auto a : q) std::cout<<a.dump() <<std::endl;

    
    
    return 0;
}