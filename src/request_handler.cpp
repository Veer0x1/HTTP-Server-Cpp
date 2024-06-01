#include "request_handler.h"
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <regex>
#include <unistd.h>
#include <cstdlib>
#include <sstream>
#include <filesystem>
#include <fstream>

void handleRootRoute(int client_fd)
{
    // std::string responseBody = "<html><body><h1>But In The End, Every Death Is Just A New Beginning.</h1></body></html>";
    // std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + std::to_string(responseBody.size()) + "\n\n" + responseBody;
    std::string response = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, response.c_str(), response.size(), 0);
}

void handleEchoRoute(int client_fd, std::string &path)
{
    std::string content = path.substr(6);
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n" + content;
    send(client_fd, response.c_str(), response.size(), 0);
}

void handleUserAgentRoute(int client_fd, char *buffer, int bytesReceived)
{
    std::string request;
    for (int i = 0; i < bytesReceived; i++)
    {
        request += buffer[i];
    }
    std::cout << "Request received idher is: " << std::endl;
    std::cout << request << std::endl;
    std::string userAgentHeader = "User-Agent: ";
    size_t ua_start = request.find(userAgentHeader);

    // If the User-Agent header is found
    if (ua_start != std::string::npos)
    {
        size_t ua_end = request.find("\r\n", ua_start);
        std::string userAgent = request.substr(ua_start + userAgentHeader.size(), ua_end - ua_start - userAgentHeader.size());
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + std::to_string(userAgent.size()) + "\r\n\r\n" + userAgent;
        send(client_fd, response.c_str(), response.size(), 0);
    }
    else
    {
        handleNotFound(client_fd);
    }
}

void handleNotFound(int client_fd)
{
    std::string message = "HTTP/1.1 404 Not Found\r\n\r\n";
    send(client_fd, message.c_str(), message.size(), 0);
}

void handleFilesRoute(int client_fd, char **argv, std::string &path, char *method, char *buffer)
{
    std::string directory = argv[2];
    std::string filename = path.substr(7);
    std::string filePath = directory + filename;

    if (strcmp(method, "GET") == 0)
    {
        if (std::filesystem::exists(filePath))
        {
            std::ifstream file(filePath);
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n" + content;
            send(client_fd, response.c_str(), response.size(), 0);
        }
        else
        {
            handleNotFound(client_fd);
        }
    }
    else if (strcmp(method, "POST") == 0)
    {
        char *body = strstr(buffer, "\r\n\r\n");
        if (body != NULL)
        {
            body += 4; // skips the "\r\n\r\n"

            std::ofstream file(filePath);
            file << body;
            file.close();

            std::string response = "HTTP/1.1 201 Created\r\n\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
        }
        else
        {
            std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
            send(client_fd, response.c_str(), response.size(), 0);
        }
    }
    else
    {
        handleNotFound(client_fd);
    }
}

void handleClient(int client_fd, char **argv, int argc)
{
    char buffer[4096];
    int bytesReceived = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    std::cout << "Received: " << buffer << std::endl;

    char bufferCopy[4096];
    strcpy(bufferCopy, buffer);

    char *method = strtok(bufferCopy, " ");
    std::cout << "Method: " << method << std::endl;

    std::string path = strtok(NULL, " ");
    std::cout << "Path: " << path << std::endl;

    std::regex pattern("^/echo/.+$");
    std::regex filesPattern("^/files/[^/]+$");

    if (path == "/")
    {
        handleRootRoute(client_fd);
    }
    else if (std::regex_match(path, pattern))
    {
        handleEchoRoute(client_fd, path);
    }
    else if (path == "/user-agent")
    {
        handleUserAgentRoute(client_fd, buffer, bytesReceived);
    }
    else if (std::regex_match(path, filesPattern) and argc == 3 && strcmp(argv[1], "--directory") == 0)
    {
        handleFilesRoute(client_fd, argv, path, method, buffer);
    }
    else
    {
        handleNotFound(client_fd);
    }
    close(client_fd);
}