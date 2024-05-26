#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define debug(x) std::cerr << #x << " = " << x << std::endl;

int main(int argc, char **argv)
{

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
  {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting REUSE_PORT
  // ensures that we don't run into 'Address already in use' errors
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

  char buffer[1024];
  recv(client_fd, buffer, 1024, 0);
  std::cout << "Received: " << buffer << std::endl;

  std::strtok(buffer, " ");
  std::string path = std::strtok(NULL, " ");
  std::cout << "Path: " << path << std::endl;

  if(path == "/"){
    std::string message = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd,message.c_str(),message.size(),0);
  }else {
    std::string message = "HTTP/1.1 404 Not Found\r\n\r\n";
    send(client_fd,message.c_str(),message.size(),0);
  }

  
  
  close(server_fd);

  return 0;
}
