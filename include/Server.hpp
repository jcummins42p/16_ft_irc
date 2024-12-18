/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/18 21:07:38 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <algorithm>
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
    Server(int port, const std::string &in_name, const std::string &in_pass);
	Server(const Server &other);

	//	poll fd str
    struct pollfd fds[MAX_CLIENTS + 1];

	const std::string				_name;
	bool 							_running;
    int 							server_fd;
    unsigned int 					hashed_pass;
    std::map<int, Client *>			clients;
    std::map<std::string, Channel*> channels;
	std::ofstream 					logFile;

	// need to send and receive from this after polling rather than sending messages direct to fd with send()
	// keep filling this until it's output in the polling
	std::map<int, std::string > inBuffs;
	std::map<int, std::vector<std::string> > outBuffs;

	std::string handleAuth(int client_fd, const std::string &in_pass);
    void acceptClient(struct pollfd *fds);
	void handleDisconnect(int client_fd, int bytes_received);
    void handleClient(int client_fd);
	void checkAuthentication(int client_fd);
	void checkRegistration(int client_fd);
    void processMessage(int client_fd, const std::string &message);
	void broadcastMessage(const std::string &message);

	Channel *createChannel(int client_fd, std::string chName, const std::string &passwd);

	// Command handler type definition and map declaration
	typedef void (Server::*ServCommandHandler)(int, std::istringstream &);
	std::map<std::string, ServCommandHandler> commandHandlers;
    // Command handlers
    void handleNickCommand(int client_fd, std::istringstream &iss);
    void handleUserCommand(int client_fd, std::istringstream &iss);
    void handleJoinCommand(int client_fd, std::istringstream &iss);
    void handlePartCommand(int client_fd, std::istringstream &iss);
	bool sendMsgToChannel(int client_fd, const std::string &target, const std::string &msg);
    void handlePrivmsgCommand(int client_fd, std::istringstream &iss);
    void handleQuitCommand(int client_fd, std::istringstream &iss);
    void handleTopicCommand(int client_fd, std::istringstream &iss);
    void handleModeCommand(int client_fd, std::istringstream &iss);
    void handleKickCommand(int client_fd, std::istringstream &iss);
    void handleBanCommand(int client_fd, std::istringstream &iss);
    void handleInviteCommand(int client_fd, std::istringstream &iss);
	void handleListCommand(int client_fd, std::istringstream &iss);
	void handleDieCommand(int client_fd, std::istringstream &iss);
	void handleCapCommand(int client_fd, std::istringstream &iss);
	void handlePassCommand(int client_fd, std::istringstream &iss);
	void handlePingCommand(int client_fd, std::istringstream &iss);

	Logger log; // NOT const

	//	Wrapper for send to automaticall calculate size
	std::string getPrefix(int sender_fd);

public:
	//	Singleton server startup
	static Server *getInstance(int port, const std::string &in_name, const std::string &in_pass);
    ~Server(void);
    void run();

	int	getFd( void );
	std::string serverName( void );
	void sendString(int sender_fd, int client_fd, const std::string &message) ;

	const int	&getClientFd(const std::string &search) const;
	Client 		*getClient(const std::string &search) const;
	Client 		*getClient(const int &fd) const;
	Client 		&getClientRef(const std::string &search) const;
	Client		&getClientRef(const int &fd) const;
	Channel		*getChannel(const std::string &search) const;
	Channel		&getChannelRef(const std::string &search) const;

	void sendMessages(struct pollfd &fd);
	void removeChannel( const Channel &channel );
};

#endif
