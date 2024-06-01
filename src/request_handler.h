#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>

void handleRootRoute(int client_fd);
void handleEchoRoute(int client_fd, std::string &path);
void handleUserAgentRoute(int client_fd, char *buffer, int bytesReceived);
void handleNotFound(int clinet_fd);
void handleFilesRoute(int client_fd, char **argv, std::string &path,char* method,char* buffer);

void handleClient(int client_fd,char** argv,int argc);

#endif