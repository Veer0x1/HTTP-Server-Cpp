#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <regex>

#define debug(x) std::cerr << #x << " = " << x << std::endl;

int main(int argc, char **argv)
{
  // creating a socket file descriptor with IPv4, TCP, and default protocol
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
  {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // set the SO_REUSEPORT option
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
  {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
  {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0)
  {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
  std::cout << "Client connected\n";

  char buffer[4096];
  int bytesReceived = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
  std::cout << "Received: " << buffer << std::endl;

  std::strtok(buffer, " ");
  std::string path = std::strtok(NULL, " ");
  std::cout << "Path: " << path << std::endl;

  std::regex pattern("^/echo/.+$");

  if (path == "/")
  {
    std::string message = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, message.c_str(), message.size(), 0);
  }
  else if (std::regex_match(path, pattern))
  {
    std::string content = path.substr(6);
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n" + content;
    send(client_fd, response.c_str(), response.size(), 0);
  }
  else if (path == "/user-agent")
  {
    std::string request;
    for(int i = 0; i < bytesReceived; i++)
    {
      request += buffer[i];
    }
    std::string userAgentHeader = "User-Agent: ";
    size_t ua_start = request.find(userAgentHeader);

    //If the User-Agent header is found
    if(ua_start != std::string::npos){
      size_t ua_end = request.find("\r\n", ua_start);
      std::string userAgent = request.substr(ua_start + userAgentHeader.size(), ua_end - ua_start - userAgentHeader.size());
      std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(userAgent.size()) + "\r\n\r\n" + userAgent;
      send(client_fd, response.c_str(), response.size(), 0);
    }
    else{
      std::string message = "HTTP/1.1 404 Not Found\r\n\r\n";
      send(client_fd, message.c_str(), message.size(), 0);
    }

    std::cout << request << std::endl;
  }
  else
  {
    std::string message = "HTTP/1.1 404 Not Found\r\n\r\n";
    send(client_fd, message.c_str(), message.size(), 0);
  }

  return 0;
}
