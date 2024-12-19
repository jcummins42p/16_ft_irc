/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/19 23:39:08 by jcummins         ###   ########.fr       */
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

//	Client Commands

void Server::acceptClient(struct pollfd* fds) {
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd < 0) {
		log.error("Failed to accept client.");
		return;
	}

	log.info("Client connected: fd " + intToString(client_fd));
	Client *new_client = new Client(client_fd, *this);
	clients[client_fd] = new_client;

	bool assigned = false;
	for (int i = 1; i <= MAX_CLIENTS; i++) {
		if (fds[i].fd == -1) {
			fds[i].fd = client_fd;
			fds[i].events = POLLIN;
			assigned = true;
			break;
		}
	}
	if (!assigned) {
		log.error("Too many clients. Rejecting client fd " + intToString(client_fd));
		delete new_client;
		clients.erase(client_fd);
		close(client_fd);
	}
}

void Server::handleDisconnect(int client_fd, int bytes_received) {
	if (bytes_received == 0) {
		log.info("Client disconnected: fd " + intToString(client_fd));
	} else {
		log.error("Received error on client: fd " + intToString(client_fd));
	}
	close(client_fd);
	delete clients[client_fd];
	clients.erase(client_fd);
	inBuffs.erase(client_fd);
	outBuffs.erase(client_fd);

	for (int i = 1; i <= MAX_CLIENTS; i++) {
		if (fds[i].fd == client_fd) {
			fds[i].fd = -1;
			fds[i].events = 0;
			break;
		}
	}
}

std::string Server::handleAuth(int client_fd, const std::string &in_pass)
{
	std::string output;

	if (!clients[client_fd]->isAuthenticated()) {
		unsigned int in_hashed_pass = hashSimple(in_pass);

		if (in_hashed_pass != this->hashed_pass)
			throw std::runtime_error("464 " + clients[client_fd]->getNick()
					+ " :Password incorrect");
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
			throw std::runtime_error("421 " + client.getNick() + " CMD :Unknown command '" + command + "'");
	}
	catch (std::exception &e) {
		sendString(getFd(), client_fd, std::string(e.what()));
		log.error("ProcessMessage: client fd " + intToString(client_fd) + ": " + std::string(e.what()));
	}
}

//	recv receives a message from a socket
void Server::handleClient(int client_fd) {
	char buffer[BUFFER_SIZE] = {0};
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_received <= 0)
		return (handleDisconnect(client_fd, bytes_received));

	buffer[bytes_received] = '\0';
	inBuffs[client_fd] += buffer;

	size_t pos;
	while ((pos = inBuffs[client_fd].find("\n")) != std::string::npos) {
		// Extract and process one complete message at a time
		pos = (pos > 0 && inBuffs[client_fd][pos - 1] == '\r') ? pos - 1 : pos;
		std::string message = inBuffs[client_fd].substr(0, pos);
		inBuffs[client_fd].erase(0, pos + 1);
		if (!message.empty()) {
			log.info("Received message from fd " + intToString(client_fd) + ": " + message);
			processMessage(client_fd, message);
		}
	}
	inBuffs[client_fd].erase();
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
