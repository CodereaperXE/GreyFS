#include "network.hpp"

//server implementation
//constructor
Network::Server::Server(int port) : port(port){
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

//destructor
Network::Server::~Server(){
    if(clientSockFd > 0) close(clientSockFd);
    if(serverSockFd > 0) close(serverSockFd);
    std::cout<<"server off"<<std::endl;
}

json Network::Server::AcceptReq(){
    // int clientFd = accept(serverSockFd,(struct sockaddr*)&clientAddr,&clientLen);
    
//    while(clientSockFd <=0)
    clientSockFd = accept(serverSockFd,(struct sockaddr*)&clientAddr,&clientLen);
    
   
    
    if (clientSockFd < 0) {

         std::cerr << "Error accepting connection: " << strerror(errno) << " (errno: " << errno << ")\n";
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        std::cerr << "Non-blocking mode and no connections are pending\n";
    } else if (errno == EINTR) {
        std::cerr << "Interrupted by a signal before a connection arrived\n";
    } else if (errno == ECONNABORTED) {
        std::cerr << "Connection was aborted before it could be accepted\n";
    } else if (errno == EMFILE || errno == ENFILE) {
        std::cerr << "File descriptor or system resource limit reached\n";
    } else {
        std::cerr << "Unknown error: " << strerror(errno) << "\n";
    }


        std::cerr << "Server side error accepting connection\n";
        close(serverSockFd);
        return json{{"server side error accepting connection",1}};
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

int Network::Server::SendResponse(json response){
    // const char* response = "Message received!";
    send(clientSockFd, response.dump().c_str(), strlen(response.dump().c_str()), 0);
    close(clientSockFd);
    return 1;
}


//client implementation

Network::Client::Client(std::string address,int port){
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

Network::Client::~Client(){
    if(sockFd > 0) close(sockFd);
}

json Network::Client::SendData(json data){
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


