/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/26 17:42:12 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCSERVER_HPP
# define IRCSERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"

#include <iostream>
#include <map>
#include <sstream>
#include <arpa/inet.h>

class Server {
	public:
		Server(int port, std::string in_pass);
		~Server();
		void run();

	private:
		int								server_fd;
		std::map<int, Client*>			clients;
		std::map<std::string, Channel*>	channels;
		unsigned int					hashed_pass;

		void acceptClient(struct pollfd* fds);
		void handleClient(int client_fd);
		void processMessage(int client_fd, const std::string& message);
		void handleNickCommand(int client_fd, std::istringstream& iss);
		void handleUserCommand(int client_fd, std::istringstream& iss);
		void handleJoinCommand(int client_fd, std::istringstream& iss);
		void handlePartCommand(int client_fd, std::istringstream& iss); // what's the use?
		void handlePrivmsgCommand(int client_fd, std::istringstream& iss);
		void handleQuitCommand(int client_fd);
		void handleTopicCommand(int client_fd, std::istringstream& iss);
		void handleModeCommand(int client_fd, std::istringstream& iss);
		void handleKickCommand(int client_fd, std::istringstream& iss);
		void handleInviteCommand(int client_fd, std::istringstream& iss);
};

#endif
