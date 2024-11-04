/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ircserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-04 13:00:32 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/04 15:29:12 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef IRCSERV_HPP
#define IRCSERV_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 512

class Client;  // Forward declaration for client class

class IRCServer {
public:
	IRCServer(int port, const std::string& password);
	~IRCServer();
	bool setupServer();
	void runServer();

private:
	int port;
	std::string password;
	int server_fd;
	struct sockaddr_in server_addr;
	std::vector<pollfd> poll_fds;
	std::map<int, Client*> clients;  // Map for client socket to Client objects

	void acceptNewClient();
	void handleClient(int client_fd);
	void removeClient(int client_fd);
};

class Client {
public:
	Client(int socket_fd);
	~Client();

	int getSocket() const;
	void sendMessage(const std::string& message);

private:
	int socket_fd;
	std::string nickname;
};

#endif
