/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/18 21:48:20 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <arpa/inet.h>
#include <ctime> // For time functions

//	General functions

void Server::broadcastMessage(const std::string &message) {
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
		sendString(server_fd, it->second->getFd(), message);
}

void Server::sendMessages(struct pollfd &fd)
{
	if (fd.revents & POLLOUT) {
		int client_fd = fd.fd;
		if (!outBuffs[client_fd].empty()) {
			const std::string &message = outBuffs[client_fd].front();
			// this is the only place we use send!
			ssize_t bytes_out = send(client_fd, (message + "\n").c_str(), message.size() + 1, 0);
			if (bytes_out > 0) {
				log.info("Message sent to client " + intToString(client_fd) + ": " + message);
				outBuffs[client_fd].erase(outBuffs[client_fd].begin());
			} else if (bytes_out < 0) {
				log.error("Failed to send message to client " + intToString(client_fd) + ": " + message);
			}
		}
		if (outBuffs[client_fd].empty()) {
			fd.events &= ~POLLOUT;
		}
	}
}

//	Get the correct message prefix based on sender fd
std::string Server::getPrefix(int sender_fd) {
	std::string prefix;

	if (sender_fd == server_fd)
		prefix = ":" + serverName() + " ";
	else {
		Client &client = getClientRef(sender_fd);
		prefix = ":" + client.getNick() + "!" + client.getUser() + " ";
	}
	return (prefix);
}

//	Add message to the server's message out buffer for that client
void Server::sendString(int sender_fd, int target_fd, const std::string &message) {
	try {
		outBuffs[target_fd].push_back( getPrefix(sender_fd) + message );

		for (int i = 0; i <= MAX_CLIENTS; ++i) {
			if (fds[i].fd == target_fd) {
				fds[i].events |= POLLOUT;
				break;
			}
		}
	}
	catch ( std::exception &e ) {
		std::cerr 	<< "Error adding message to buffer of client " << target_fd
					<< ": " << e.what() << std::endl;
	}
}

//	Client Commands

void Server::acceptClient(struct pollfd* fds) {
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd < 0) {
		log.error("Failed to accept client.");
		return;
	}

	log.info("Client connected: fd " + intToString(client_fd));
	clients[client_fd] = new Client(client_fd, *this);

	for (int i = 1; i <= MAX_CLIENTS; i++) {
		if (fds[i].fd == -1) {
			fds[i].fd = client_fd;
			fds[i].events = POLLIN;
			break;
		}
	}
}

void Server::handleDisconnect(int client_fd, int bytes_received) {
	if (bytes_received == 0) {
		log.info("Client disconnected: fd " + intToString(client_fd));
	} else {
		log.error("Received error on client: fd " + intToString(client_fd));
	}
	close(client_fd);
	clients.erase(client_fd);
	inBuffs.erase(client_fd);
}

std::string Server::handleAuth(int client_fd, const std::string &in_pass)
{
	std::string output;

	if (!clients[client_fd]->isAuthenticated()) {
		unsigned int in_hashed_pass = hashSimple(in_pass);

		if (in_hashed_pass != this->hashed_pass)
			throw std::runtime_error("Password incorrect");
		clients[client_fd]->setAuthenticated();
		output = "Password accepted";
		return (output);
	}
	return ("Already authenticated");
}

void Server::checkAuthentication(int client_fd) {
	Client &client = getClientRef(client_fd);
	if (client.isAuthenticated())
		return ;
	std::string message = "451 " + client.getNick() + " :You need to authenticate PASS";
	throw std::runtime_error(message);
}

void Server::checkRegistration(int client_fd) {
	Client &client = getClientRef(client_fd);
	if (client.isRegistered())
		return ;
	std::string message = "451 " + client.getNick() + " :You need to register ";
	if (client.getNick().empty() || client.getNick() == "*")
		message += "NICK ";
	if (client.getUser().empty())
		message += "USER ";
	throw std::runtime_error(message);
}

static bool requiresAuthentication(const std::string &command) {
	std::string permitted[6] = {
		"CAP", "PASS", "PING", "QUIT" };
	return std::find(permitted, permitted + 6, command) == permitted + 6;
}

static bool requiresRegistration(const std::string &command) {
	std::string permitted[6] = {
		"CAP", "PASS", "USER", "NICK", "PING", "QUIT" };
	return std::find(permitted, permitted + 6, command) == permitted + 6;
}

Channel *Server::createChannel(int client_fd, std::string chName, const std::string &passwd) {
	if (chName.empty())
		chName = "#Default";
	if (getChannel(chName))
		throw std::invalid_argument(chName + " already in use ");
	Channel::validateName(chName);
	Channel *output = new Channel(*this, chName, *clients[client_fd], passwd);

	channels[output->getName()] = output;  // Add the new channel to the map
	sendString(client_fd, client_fd, "JOIN " + output->getName());
	log.info( getClientRef(client_fd).getNick() + " (fd " + intToString(client_fd)
			+ ") created channel " + output->getName());
	return (output);
}

void	Server::removeChannel( const Channel &channel ) {
	if (!channel.getName().empty()) {
		std::cout << "Attempting to remove " + channel.getName();
		channels.erase(channel.getName());
	}
}

void Server::processMessage(int client_fd, const std::string& input) {
	Client &client = getClientRef(client_fd);
	std::istringstream iss(input);
	std::string command, err_message;
	iss >> command;

	//	Removed the if/else forest and use function pointers in a map instead
	try {
		if (command.empty())
			return;
		if (requiresAuthentication(command))	//	Throws if command requires authentication
			checkAuthentication(client_fd);
		if (requiresRegistration(command))	//	Throws if command requires registration
			checkRegistration(client_fd);
		std::map<std::string, ServCommandHandler>::iterator it = commandHandlers.find(command);
		if (it != commandHandlers.end())
			(this->*(it->second))(client_fd, iss);
		else
			throw std::runtime_error("Unknown command '" + command + "'");
	}
	catch (std::exception &e) {
		sendString(getFd(), client_fd, "421 " + client.getNick() + " CMD :" + std::string(e.what()));
		log.error("ProcessMessage: client fd " + intToString(client_fd) + ": " + std::string(e.what()));
	}
}

//	recv receives a message from a socket
void Server::handleClient(int client_fd) {
	char buffer[BUFFER_SIZE];
	for (unsigned int i = 0; i < BUFFER_SIZE; i++)
		buffer[i] = '\0';
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_received <= 0)
		handleDisconnect(client_fd, bytes_received);

	if (buffer[bytes_received - 1] != '\n') {
		buffer[bytes_received] = '\n';
		buffer[bytes_received] = '\0';
	}
	else
		buffer[bytes_received] = '\0';
	inBuffs[client_fd] += buffer;

	size_t pos;
	while ((pos = inBuffs[client_fd].find("\n")) != std::string::npos) {
		// Extract and process one complete message at a time
		std::string message = inBuffs[client_fd].substr(0, pos);
		inBuffs[client_fd].erase(0, pos + 2);
		log.info("Received message from fd " + intToString(client_fd) + ": " + message);
		//	need to handle messages which end with \r\n to comply with irc
		if (!message.empty() && message[message.size() - 1] == '\r' )
			message[message.size() - 1] = '\0';
		//if (!handleAuth(client_fd, message))
		processMessage(client_fd, message);
	}
}

void Server::run() {
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;

	for (int i = 1; i <= MAX_CLIENTS; i++) {
		fds[i].fd = -1;
		fds[i].events = POLLIN;
		fds[i].revents = 0;
	}
	log.info("Server is running. Waiting for connections...");
	while (_running) { // waits for results on the monitored file descriptors
		int ret = poll(fds, MAX_CLIENTS + 1, -1);
		if (ret < 0) {
			log.error("poll() failed");
			break;
		}
		for (int i = 0; i <= MAX_CLIENTS; i++) {
			// check for readable events POLLIN
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == server_fd)
					acceptClient(fds);
				else
					handleClient(fds[i].fd);
			}
			sendMessages(fds[i]);
		}
	}
}
