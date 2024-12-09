/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/09 17:39:10 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <poll.h>
#include <iostream>
#include <cstdio>
#include "Client.hpp"
#include "Channel.hpp"
#include "Logger.hpp"
#include "ft_irc.hpp" // Ensure it includes the MAX_CLIENTS definition

#define BUFFER_SIZE 512 // Keep this here unless it's also defined elsewhere

class Server {
private:
	// static pointer to the instance of the class
	static Server *instancePtr;
	//	Private constructor to prevent preation of multiple servers
    Server(int port, const std::string& in_pass);
	Server(const Server &other);

	//	poll fd str
    struct pollfd fds[MAX_CLIENTS + 1];

    int server_fd;
    unsigned int hashed_pass;
    std::map<int, Client*> clients;
    std::map<std::string, Channel*> channels;
	std::ofstream logFile;

	// need to send and receive from this after polling rather than sending messages direct to fd with send()
	// keep filling this until it's output in the polling
	std::map<int, std::string > inBuffs;
	std::map<int, std::vector<std::string> > outBuffs;

    void acceptClient(struct pollfd* fds);
	void handleDisconnect(int client_fd, int bytes_received);
    void handleClient(int client_fd);
	int handleAuth(int client_fd, const std::string &message);
	void promptRegistration(int client_fd);
    void processMessage(int client_fd, const std::string& message);

	Channel *createChannel(int client_fd, std::string chName, std::string passwd);

    // Command handlers
    void handleNickCommand(int client_fd, std::istringstream& iss);
    void handleUserCommand(int client_fd, std::istringstream& iss);
    void handleJoinCommand(int client_fd, std::istringstream& iss);
    void handlePartCommand(int client_fd, std::istringstream& iss);
    void handlePrivmsgCommand(int client_fd, std::istringstream& iss);
    void handleQuitCommand(int client_fd);
    void handleTopicCommand(int client_fd, std::istringstream& iss);
    void handleModeCommand(int client_fd, std::istringstream& iss);
    void handleKickCommand(int client_fd, std::istringstream& iss);
    void handleInviteCommand(int client_fd, std::istringstream& iss);

	Logger log; // NOT const

	//	Wrapper for send to automaticall calculate size

public:
	//	Singleton server startup
	static Server *getInstance(int port, const std::string &in_pass);
    ~Server(void);
    void run();

	void sendString(int client_fd, const std::string &message) ;

	Client *getClient(const std::string &search);
	Client *getClient(const int &fd);
	Client &getClientRef(const std::string &search);
	Client &getClientRef(const int &fd);
	Channel *getChannel(const std::string &search);
	Channel &getChannelRef(const std::string &search);

	void sendMessages(struct pollfd &fd);
};

#endif
