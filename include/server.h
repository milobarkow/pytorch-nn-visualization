#include <chrono>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Server {
public:
  Server(int port);
  void start();
  const json &getReceivedData() const;

private:
  int port;
  int serverSocket;
  json receivedData;

  void processClientData(int clientSocket);
};

Server::Server(int port) : port(port), serverSocket(-1) {}

void Server::start() {
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    std::cerr << "Error creating socket" << std::endl;
    return;
  }

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) <
      0) {
    std::cerr << "Error binding socket" << std::endl;
    return;
  }

  if (listen(serverSocket, 10) == 0) {
  } else {
    std::cerr << "Error listening" << std::endl;
    return;
  }

  while (true) {
    struct sockaddr_in newAddr;
    socklen_t addrSize = sizeof(newAddr);
    int newSocket = accept(serverSocket, (struct sockaddr *)&newAddr,
                           &addrSize); // Accept the connection

    if (newSocket >= 0) {
      processClientData(newSocket);
      close(newSocket);
    } else {
      std::cerr << "Error accepting a connection" << std::endl;
    }
  }
}

void Server::processClientData(int clientSocket) {
  char buffer[1024];
  int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
  if (bytesRead > 0) {
    buffer[bytesRead] = '\0';

    try {
      json received_data = json::parse(buffer);
      receivedData = received_data; // Store the received data
      std::cout << "Received JSON data:\n";
    } catch (const std::exception &e) {
      std::cerr << "Failed to parse JSON data: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "Error receiving data" << std::endl;
  }
}

const json &Server::getReceivedData() const { return receivedData; }
