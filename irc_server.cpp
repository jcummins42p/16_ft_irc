/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc_server.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024-11-05 14:48:02 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

class Client {
public:
    int fd;
    std::string nickname;
    Client(int fd) : fd(fd) {}
};

class Channel {
public:
    std::string name;
    std::set<int> clients;
    std::set<int> invitedClients;
    int operatorFd;
    bool inviteOnly;

    Channel(const std::string& name, int firstClientFd)
        : name(name), operatorFd(firstClientFd), inviteOnly(false) {
        clients.insert(firstClientFd);
    }

    void addClient(int client_fd) {
        clients.insert(client_fd);
    }

    void removeClient(int client_fd) {
        clients.erase(client_fd);
        invitedClients.erase(client_fd);
    }

    bool isOperator(int client_fd) const {
        return client_fd == operatorFd;
    }

    bool isInvited(int client_fd) const {
        return !inviteOnly || invitedClients.count(client_fd);
    }

    void inviteClient(int client_fd) {
        invitedClients.insert(client_fd);
    }

    void setInviteOnly(bool value) {
        inviteOnly = value;
    }

    void sendMessage(const std::string& message, int sender_fd) {
        for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) {
            int client_fd = *it;
            if (client_fd != sender_fd) {
                send(client_fd, message.c_str(), message.size(), 0);
            }
        }
    }
};

class IRCServer {
private:
    int server_fd;
    std::map<int, Client*> clients;
    std::map<std::string, Channel*> channels;

public:
    IRCServer(int port);
    ~IRCServer();
    void run();

private:
    void handleClientMessage(int client_fd);
    void handleJoinCommand(int client_fd, std::istringstream& params);
    void handleKickCommand(int client_fd, std::istringstream& params);
    void handleModeCommand(int client_fd, std::istringstream& params);
    void handleInviteCommand(int client_fd, std::istringstream& params);
    void handlePrivmsgCommand(int client_fd, std::istringstream& params);
};

IRCServer::IRCServer(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 10);
}

IRCServer::~IRCServer() {
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        delete it->second;
    }
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        delete it->second;
    }
    close(server_fd);
}

void IRCServer::run() {
    struct pollfd fds[MAX_CLIENTS];
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while (true) {
        poll(fds, MAX_CLIENTS, -1);
        if (fds[0].revents & POLLIN) {
            int client_fd = accept(server_fd, NULL, NULL);
            clients[client_fd] = new Client(client_fd);
        }

        for (int i = 1; i < MAX_CLIENTS; ++i) {
            if (fds[i].revents & POLLIN) {
                handleClientMessage(fds[i].fd);
            }
        }
    }
}

void IRCServer::handleClientMessage(int client_fd) {
    char buffer[BUFFER_SIZE];
    int bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (bytes <= 0) {
        close(client_fd);
        clients.erase(client_fd);
        return;
    }

    std::istringstream message(buffer);
    std::string command;
    message >> command;

    if (command == "JOIN") {
        handleJoinCommand(client_fd, message);
    } else if (command == "KICK") {
        handleKickCommand(client_fd, message);
    } else if (command == "MODE") {
        handleModeCommand(client_fd, message);
    } else if (command == "INVITE") {
        handleInviteCommand(client_fd, message);
    } else if (command == "PRIVMSG") {
        handlePrivmsgCommand(client_fd, message);
    }
}

void IRCServer::handleJoinCommand(int client_fd, std::istringstream& params) {
    std::string channel_name;
    params >> channel_name;

    Channel* channel;
    if (channels.find(channel_name) == channels.end()) {
        channel = new Channel(channel_name, client_fd);
        channels[channel_name] = channel;
        std::string operatorMessage = "You are now the operator of " + channel_name + "\n";
        send(client_fd, operatorMessage.c_str(), operatorMessage.size(), 0);
    } else {
        channel = channels[channel_name];
    }

    if (channel->isInvited(client_fd)) {
        channel->addClient(client_fd);
    } else {
        std::string response = "You're not invited to this channel.\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
}

void IRCServer::handleKickCommand(int client_fd, std::istringstream& params) {
    std::string channel_name, target_nick;
    params >> channel_name >> target_nick;

    if (channels.find(channel_name) == channels.end()) return;

    Channel* channel = channels[channel_name];
    if (!channel->isOperator(client_fd)) return;

    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->nickname == target_nick) {
            int target_fd = it->first;
            channel->removeClient(target_fd);
            std::string message = target_nick + " was kicked from " + channel_name + "\n";
            channel->sendMessage(message, client_fd);
            break;
        }
    }
}

void IRCServer::handleModeCommand(int client_fd, std::istringstream& params) {
    std::string channel_name, mode;
    params >> channel_name >> mode;

    if (channels.find(channel_name) == channels.end()) return;

    Channel* channel = channels[channel_name];
    if (!channel->isOperator(client_fd)) return;

    if (mode == "+i") {
        channel->setInviteOnly(true);
    } else if (mode == "-i") {
        channel->setInviteOnly(false);
    }
}

void IRCServer::handleInviteCommand(int client_fd, std::istringstream& params) {
    std::string target_nick, channel_name;
    params >> target_nick >> channel_name;

    if (channels.find(channel_name) == channels.end()) return;

    Channel* channel = channels[channel_name];
    if (!channel->isOperator(client_fd)) return;

    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->nickname == target_nick) {
            channel->inviteClient(it->first);
            std::string response = target_nick + " has been invited to " + channel_name + "\n";
            send(client_fd, response.c_str(), response.size(), 0);
            break;
        }
    }
}

void IRCServer::handlePrivmsgCommand(int client_fd, std::istringstream& params) {
    std::string target, message;
    params >> target;
    std::getline(params, message);

    if (channels.find(target) != channels.end()) {
        Channel* channel = channels[target];
        if (channel->clients.count(client_fd)) {
            channel->sendMessage(message, client_fd);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0) {
        std::cerr << "invalid port number." << std::endl;
        return 1;
    }

    IRCServer server(port);
    server.run();

    return 0;
}
