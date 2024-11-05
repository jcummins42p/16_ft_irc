/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-05 14:46:54 by pyerima           #+#    #+#             */
/*   Updated: 2024-11-05 14:46:54 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <poll.h>
#include <sstream>

//constructor and Destructor
Server::Server(int port) : port_(port), server_fd_(-1) {}

Server::~Server() {
    if (server_fd_ >= 0)
        close(server_fd_);
    for (std::map<int, Client*>::iterator it = clients_.begin(); it != clients_.end(); ++it)
        delete it->second;
}

//start the server
bool Server::start() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("Socket creation failed");
        return false;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return false;
    }

    if (listen(server_fd_, 5) < 0) {
        perror("Listen failed");
        return false;
    }

    std::cout << "Server started on port " << port_ << std::endl;
    return true;
}

//main server loop
void Server::run() {
    struct pollfd fds[100];
    int nfds = 1;

    fds[0].fd = server_fd_;
    fds[0].events = POLLIN;

    while (true) {
        int poll_count = poll(fds, nfds, -1);
        if (poll_count < 0) {
            perror("Poll failed");
            break;
        }

        if (fds[0].revents & POLLIN)
            acceptClient();

        for (int i = 1; i < nfds; ++i) {
            if (fds[i].revents & POLLIN) {
                char buffer[1024];
                int len = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                if (len <= 0) {
                    disconnectClient(fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    --nfds;
                } else {
                    buffer[len] = '\0';
                    handleMessage(fds[i].fd, buffer);
                }
            }
        }
    }
}

//accept new client
void Server::acceptClient() {
    int client_fd = accept(server_fd_, NULL, NULL);
    if (client_fd < 0) {
        perror("Client accept failed");
        return;
    }

    clients_[client_fd] = new Client(client_fd);
    std::cout << "Client connected: " << client_fd << std::endl;
}

//sisconnecting client
void Server::disconnectClient(int fd) {
    std::cout << "Client disconnected: " << fd << std::endl;
    delete clients_[fd];
    clients_.erase(fd);
    close(fd);
}

//for handling incoming messages
void Server::handleMessage(int fd, const std::string& message) {
    std::istringstream iss(message);
    std::string command, param1, param2;
    iss >> command >> param1 >> param2;

    if (command == "NICK")
        handleNick(fd, param1);
    else if (command == "JOIN")
        handleJoin(fd, param1);
    else if (command == "PRIVMSG")
        handlePrivMsg(fd, param1, param2);
}

//command handlers
void Server::handleNick(int fd, const std::string& nickname) {
    clients_[fd]->setNickname(nickname);
}

void Server::handleJoin(int fd, const std::string& channel) {
    channels_[channel].insert(fd);
    std::cout << "Client " << fd << " joined channel " << channel << std::endl;
}

void Server::handlePrivMsg(int fd, const std::string& recipient, const std::string& message) {
    for (std::set<int>::iterator it = channels_[recipient].begin(); it != channels_[recipient].end(); ++it) {
        if (*it != fd)
            send(*it, message.c_str(), message.size(), 0);
    }
}

//client class
Client::Client(int fd) : fd_(fd) {}

Client::~Client() {}

void Client::setNickname(const std::string& nickname) {
    nickname_ = nickname;
}

std::string Client::getNickname() const {
    return nickname_;
}

int Client::getFd() const {
    return fd_;
}
