/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/05 18:40:30 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCSERVER_HPP
# define IRCSERVER_HPP

#include "Client.hpp"

#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdlib.h>

class IRCServer {
	public:
		IRCServer(int port);
		~IRCServer();
		void run();

	private:
		int server_fd;
		std::map<int, Client*> clients;
		std::map<std::string, Channel*> channels;

		void acceptClient(struct pollfd* fds);
		void handleClient(int client_fd);
		void processMessage(int client_fd, const std::string& message);
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
};

#endif
