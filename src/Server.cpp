/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/10 18:34:27 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <arpa/inet.h>
#include <ctime> // For time functions

// Constructors / destructor

Server *Server::instancePtr = NULL;

Server *Server::getInstance(int port, const std::string &in_pass) {
	if (instancePtr == NULL) {
		instancePtr = new Server(port, in_pass);
		return instancePtr;
	}
	return instancePtr;
}

Server::Server(int port, const std::string& in_pass) {
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	hashed_pass = hashSimple(in_pass);

	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(server_fd, 5);

	// Log file now opened in Logger class constructor
	log.info("Server initialized on port " + intToString(port));
}

Server::~Server(void) {
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		close(it->first);
		delete it->second;
	}

	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		delete it->second;
	}
	close(server_fd);

	log.info("Server shutting down.");
	if (logFile.is_open()) {
		logFile.close();
	}
}

//	General functions

void Server::run() {
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;

	for (int i = 1; i <= MAX_CLIENTS; i++) {
		fds[i].fd = -1;
	}

	log.info("Server is running. Waiting for connections...");
	while (true) { // waits for results on the monitored file descriptors
		int ret = poll(fds, MAX_CLIENTS + 1, -1);
		if (ret < 0) {
			log.error("poll() failed");
			break;
		}
		for (int i = 0; i <= MAX_CLIENTS; i++) {
			// check for readable events POLLIN
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == server_fd) {
					acceptClient(fds);
				} else {
					handleClient(fds[i].fd);
				}
			}
			sendMessages(fds[i]);
		}
	}
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

//	Add message to the server's message out buffer for that client
void Server::sendString(int client_fd, const std::string &message) {
	try {
		outBuffs[client_fd].push_back( message );

		for (int i = 0; i <= MAX_CLIENTS; ++i) {
			if (fds[i].fd == client_fd) {
				fds[i].events |= POLLOUT;
				break;
			}
		}
	}
	catch ( std::exception &e ) {
		std::cerr 	<< "Error adding message to buffer of client " << client_fd
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
	sendString(client_fd, "Please enter the server password:");
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

//	recv receives a message from a socket
void Server::handleClient(int client_fd) {
	char buffer[BUFFER_SIZE];
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_received <= 0)
		handleDisconnect(client_fd, bytes_received);

	buffer[bytes_received] = '\0';
	inBuffs[client_fd] += buffer;

	size_t pos;
	while ((pos = inBuffs[client_fd].find("\n")) != std::string::npos) {
		// Extract and process one complete message at a time
		std::string message = inBuffs[client_fd].substr(0, pos);
		inBuffs[client_fd].erase(0, pos + 2);
		log.info("Received message from fd " + intToString(client_fd) + ": " + message);
		//	need to handle messages which end with \r\n to comply with irc
		if (!message.empty() && message[message.size() - 1] == '\r' ) {
			message[message.size() - 1] = '\0';
		}
		if (!handleAuth(client_fd, message))
			processMessage(client_fd, message);
	}
}

int Server::handleAuth(int client_fd, const std::string &message)
{
	if (!clients[client_fd]->isAuthenticated()) {
		unsigned int in_hashed_pass = hashSimple(message);

		if (in_hashed_pass != this->hashed_pass) {
			log.warn("Authentication failed for fd " + intToString(client_fd));
			sendString(client_fd, "Wrong password, try again: ");
		} else {
			clients[client_fd]->setAuthenticated();
			log.info("Client authenticated: fd " + intToString(client_fd));
			sendString(client_fd, "Authentication successful!");
		}
		return (1);
	}
	return (0);
}

void Server::promptRegistration(int client_fd) {
	std::string message = "You are not registered.\nPlease set a ";
	if (getClientRef(client_fd).getNick().empty()) {
		message += "NICK ";
		if (getClientRef(client_fd).getUser().empty())
		message += "and USER ";
	}
	else
		message += "USER ";
	message += "name.";
	sendString(client_fd, message);
}

void Server::processMessage(int client_fd, const std::string& message) {
	std::istringstream iss(message);
	std::string command;
	iss >> command;

	if (command == "NICK") {
		handleNickCommand(client_fd, iss);
	} else if (command == "USER") {
		handleUserCommand(client_fd, iss);
	} else if (!getClientRef(client_fd).isRegistered()) {
		promptRegistration(client_fd);
	} else if (command == "JOIN") {
		handleJoinCommand(client_fd, iss);
	} else if (command == "PART") {
		handlePartCommand(client_fd, iss);
	} else if (command == "PRIVMSG") {
		handlePrivmsgCommand(client_fd, iss);
	} else if (command == "QUIT") {
		handleQuitCommand(client_fd);
	} else if (command == "PING") {
		send(client_fd, "PONG :ping\n", 12, 0);
	} else if (command == "TOPIC") {
		handleTopicCommand(client_fd, iss);
	} else if (command == "MODE") {
		handleModeCommand(client_fd, iss);
	} else if (command == "KICK") {
		handleKickCommand(client_fd, iss);
	} else if (command == "INVITE") {
		handleInviteCommand(client_fd, iss);
	} else {
		log.info("Unhandled message from fd " + intToString(client_fd) + ": " + message);
	}
}

Channel *Server::createChannel(int client_fd, std::string chName, std::string passwd) {
	Channel *output = new Channel(*this, chName, *clients[client_fd], passwd);
	channels[chName] = output;  // Add the new channel to the map
	sendString(client_fd, "Made new channel '" + output->getName() + "' successfully.");
	log.info( getClientRef(client_fd).getNick() + " (fd " + intToString(client_fd)
			+ ") created channel " + output->getName());
	return (output);
}
