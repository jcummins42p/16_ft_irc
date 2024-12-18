/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConstrDestr.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/18 18:59:36 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <arpa/inet.h>
#include <ctime> // For time functions

// Constructors / destructor fro server class

Server *Server::instancePtr = NULL;

Server *Server::getInstance(int port, const std::string &in_name, const std::string &in_pass) {
	if (instancePtr == NULL) {
		instancePtr = new Server(port, in_name, in_pass);
		return instancePtr;
	}
	return instancePtr;
}

Server::Server(int port, const std::string &in_name, const std::string &in_pass) :
	_name(in_name),
	_running(true)
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	hashed_pass = hashSimple(in_pass);

	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(server_fd, 5);

	//	Initializing command map
	commandHandlers["CAP"] = &Server::handleCapCommand;
	commandHandlers["DIE"] = &Server::handleDieCommand;
	commandHandlers["LIST"] = &Server::handleListCommand;
	commandHandlers["INVITE"] = &Server::handleInviteCommand;
	commandHandlers["BAN"] = &Server::handleBanCommand;
	commandHandlers["KICK"] = &Server::handleKickCommand;
	commandHandlers["MODE"] = &Server::handleModeCommand;
	commandHandlers["TOPIC"] = &Server::handleTopicCommand;
	commandHandlers["QUIT"] = &Server::handleQuitCommand;
	commandHandlers["PRIVMSG"] = &Server::handlePrivmsgCommand;
	commandHandlers["PART"] = &Server::handlePartCommand;
	commandHandlers["JOIN"] = &Server::handleJoinCommand;
	commandHandlers["USER"] = &Server::handleUserCommand;
	commandHandlers["NICK"] = &Server::handleNickCommand;
	commandHandlers["PASS"] = &Server::handlePassCommand;
	commandHandlers["PING"] = &Server::handlePingCommand;

	// Log file now opened in Logger class constructor
	log.info("Server initialized on port " + intToString(port));
}

Server::~Server(void) {
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		close(it->first);
		delete it->second;
	}
	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
		delete it->second;
	close(server_fd);
	log.info("Server shutting down.");
	if (logFile.is_open())
		logFile.close();
}
