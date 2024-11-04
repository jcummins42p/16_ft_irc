/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-04 13:10:15 by pyerima           #+#    #+#             */
/*   Updated: 2024-11-04 13:10:15 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircserv.hpp"

IRCServer::IRCServer(int port, const std::string& password) : port(port), password(password), server_fd(-1) {}

IRCServer::~IRCServer() {
    // Replace 'auto' with explicit iterator type
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        delete it->second;  // Delete client object
        close(it->first);   // Close client socket
    }
    if (server_fd >= 0) close(server_fd);
}

bool IRCServer::setupServer() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return false;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        return false;
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK); // Set server to non-block

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return false;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return false;
    }

    pollfd pfd = { server_fd, POLLIN, 0 };
    poll_fds.push_back(pfd);
    std::cout << "IRC server started on port " << port << std::endl;
    return true;
}

void IRCServer::runServer() {
    while (true) {
        int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
        if (poll_count < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == server_fd) {
                    acceptNewClient();
                } else {
                    handleClient(poll_fds[i].fd);
                }
            }
        }
    }
}

void IRCServer::acceptNewClient() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    
    if (client_fd < 0) {
        std::cerr << "Failed to accept new client" << std::endl;
        return;
    }

    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    pollfd pfd = { client_fd, POLLIN, 0 };
    poll_fds.push_back(pfd);

    clients[client_fd] = new Client(client_fd);
    std::cout << "New client connected: " << client_fd << std::endl;
}

void IRCServer::handleClient(int client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received <= 0) {
        removeClient(client_fd);
    } else {
        std::string message(buffer, bytes_received);
        std::cout << "Received message from client " << client_fd << ": " << message << std::endl;
    }
}

void IRCServer::removeClient(int client_fd) {
    std::cout << "Removing client " << client_fd << std::endl;
    close(client_fd);
    
    // Erase the pollfd for this client
    for (std::vector<pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it) {
        if (it->fd == client_fd) {
            poll_fds.erase(it);
            break;
        }
    }

    // Delete client object and remove from clients map
    delete clients[client_fd];
    clients.erase(client_fd);
}

Client::Client(int socket_fd) : socket_fd(socket_fd) {}

Client::~Client() {
    close(socket_fd);
}

int Client::getSocket() const {
    return socket_fd;
}

void Client::sendMessage(const std::string& message) {
    send(socket_fd, message.c_str(), message.length(), 0);
}
