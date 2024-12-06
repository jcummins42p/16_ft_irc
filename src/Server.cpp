/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/06 10:09:43 by jcummins         ###   ########.fr       */
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

    log.info("Client connected: FD " + intToString(client_fd));
    clients[client_fd] = new Client(client_fd, *this);
	sendString(client_fd, "Please enter the password: ");

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        if (fds[i].fd == -1) {
            fds[i].fd = client_fd;
            fds[i].events = POLLIN;
            break;
        }
    }
}

void Server::handleClient(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            log.info("Client disconnected: FD " + intToString(client_fd));
        } else {
            log.error("Receive error on client: FD " + intToString(client_fd));
        }
        close(client_fd);
        clients.erase(client_fd);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string message(buffer);
	if (!message.empty() && message[message.length() - 1] == '\n')
		message.erase(message.length() - 1);
    log.info("Received message from FD " + intToString(client_fd) + ": " + message);

    if (!clients[client_fd]->getAuthentificated()) {
        unsigned int in_hashed_pass = hashSimple(message);

        if (in_hashed_pass != this->hashed_pass) {
            log.warn("Authentication failed for FD " + intToString(client_fd));
			sendString(client_fd, "Wrong password, try again: ");
        } else {
            clients[client_fd]->setAuthentificated();
            log.info("Client authenticated: FD " + intToString(client_fd));
			sendString(client_fd, "Authentication successful!");
        }
    } else {
        processMessage(client_fd, message);
    }
}

void Server::processMessage(int client_fd, const std::string& message) {
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    if (command == "NICK") {
        handleNickCommand(client_fd, iss);
    } else if (command == "USER") {
        handleUserCommand(client_fd, iss);
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
        log.info("Unhandled message from FD " + intToString(client_fd) + ": " + message);
    }
}

void Server::handleNickCommand(int client_fd, std::istringstream& iss)
{
	std::string in_nick;
	iss >> in_nick; // Read the nickname from the stream
	try {
		clients[client_fd]->setNick(in_nick);
		log.info("Client " + intToString(client_fd) + " set nickname to " + clients[client_fd]->getNick());
		sendString(client_fd, "Successfully set nickname to " + in_nick );
	}
	catch ( std::invalid_argument &e ) {
		log.warn("Client " + intToString(client_fd) + " failed to set empty nickname");
		sendString(client_fd, e.what());
	}
	catch ( std::exception &e ) {
		std::cerr << e.what() << std::endl;
	}
}

void Server::handleUserCommand(int client_fd, std::istringstream& iss)
{
	std::string in_username;
	iss >> in_username; // Read the username from the stream
	try {
		clients[client_fd]->setUser(in_username);
		log.info("Client " + intToString(client_fd) + " set username to " + clients[client_fd]->getUser());
		sendString(client_fd, "Successfully set username to " + in_username );
	}
	catch ( std::invalid_argument &e ) {
		log.warn("Client " + intToString(client_fd) + " failed to set empty username");
		sendString(client_fd, e.what());
	}
	catch ( std::exception &e ) {
		std::cerr << e.what() << std::endl;
	}
}

Channel *Server::createChannel(int client_fd, std::string chName, std::string passwd) {
	Channel *output = new Channel(chName, *clients[client_fd], passwd);
	channels[chName] = output;  // Add the new channel to the map
	sendString(client_fd, "Made new channel successfully.");
	log.info("Client " + intToString(client_fd) + " created channel " + output->getName());

	return (output);
}

void Server::handleJoinCommand(int client_fd, std::istringstream& iss) {
    std::string channelName;
    std::string password;
    iss >> channelName >> password;

    // Look for the channel by name
    Channel* channel = getChannel(channelName);
	// If the channel doesn't exist, create it
	if (channel == NULL) {
		try { channel = createChannel(client_fd, channelName, password); }
		catch ( std::invalid_argument &e ) {
			log.error("Client " + intToString(client_fd) + " failed to create channel " + channelName + ": " + std::string(e.what()));
			sendString(client_fd, e.what());
		}
    } else {
        // Otherwise, join the existing channel
        if (channel->joinChannel(*clients[client_fd], password)) {
			log.info("Client " + intToString(client_fd) + " joined existing channel " + channel->getName());
			sendString(client_fd, "Joined channel successfully.");
        } else {
			sendString(client_fd, "Failed to join channel.");
			log.error("Client " + intToString(client_fd) + " failed to join channel " + channel->getName());
        }
    }
}

void Server::handlePartCommand(int client_fd, std::istringstream& iss)
{
	std::string channel_name;
	iss >> channel_name;

	if (channels.find(channel_name) != channels.end()) {
		channels[channel_name]->leaveChannel(*clients[client_fd]);
		clients[client_fd]->channels.erase(channel_name);
		send(client_fd, ("Left channel: " + channel_name ).c_str(), 25, 0);
		channels[channel_name]->channelMessage(clients[client_fd]->getNick() + " has left the channel.", *clients[client_fd]);
	} else {
		sendString(client_fd, "Channel not found");
	}
}

// Why does PRIVMSG sends a message to some channel???
void Server::handlePrivmsgCommand(int client_fd, std::istringstream& iss) {
    std::string target;
    std::string msg;
    iss >> target;
    std::getline(iss, msg);

    // Trim leading whitespace from the message
    if (!msg.empty() && msg[0] == ' ')
        msg.erase(0, 1);

    // Check if the target is a channel
    if (target[0] == '#') {
        // Look for the channel by name
        Channel* channel = getChannel(target);
        if (channel) { // Send the message to all clients in the channel
            std::string formattedMsg = "Message from " + clients[client_fd]->getNick() + ": " + msg + "\n";
            channel->channelMessage(formattedMsg, *clients[client_fd]);
            return;
        } else // If the channel is not found
			return (sendString(client_fd, "Channel not found."));
    }

    // If the target is a client (not a channel), find the client by nickname
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNick() == target) {
            // Send the private message to the target client
            std::string formattedMsg = "Private message from " + clients[client_fd]->getNick() + ": " + msg;
			return (sendString(it->first, formattedMsg));
        }
    }

    // If the target client was not found
	sendString(client_fd, "Target client not found.");
}



void Server::handleQuitCommand(int client_fd)
{
	std::cout << "Client " << client_fd << " disconnected." << std::endl;
	close(client_fd); // Close the client's socket
	clients.erase(client_fd); // Remove client from the map
}

void Server::handleTopicCommand(int client_fd, std::istringstream& iss)
{
	std::string channel_name, new_topic;
	iss >> channel_name; // read channel name
	std::getline(iss, new_topic); // read the new topic

	if (new_topic.empty())
		return (sendString(client_fd, "Cannot set an empty topic!"));
	if (channels.find(channel_name) != channels.end()) {
		Channel* channel = channels[channel_name]; // get channel
		channel->setTopic(new_topic, *clients[client_fd]); // set the topic
		channel->channelMessage("TOPIC " + channel_name + " : " + new_topic + "\n", *clients[client_fd]); // notify clients of the new topic
	} else {
		sendString(client_fd, "No such channel.");
	}
}

void Server::handleModeCommand(int client_fd, std::istringstream& iss)
{
	std::string channel_name, mode;
	iss >> channel_name >> mode; // read channel name and mode
	//setting modes
	std::cout << "Client " << client_fd << " set mode for channel " << channel_name << " to " << mode << std::endl;
}

void Server::handleKickCommand(int client_fd, std::istringstream& iss) {
	std::string channel_name, target;
	iss >> channel_name >> target; // read channel name and target nickname
										//
	std::cout << "Client " << client_fd << " attempting to kick " << target << " from channel " << channel_name << std::endl;
	try {getChannelRef(channel_name).kickClient(getClientRef(target), getClientRef(client_fd));}
	catch ( std::runtime_error &e ) {
		std::cout << e.what() << std::endl;
	}
}

void Server::handleInviteCommand(int client_fd, std::istringstream& iss) {
	std::string target_nick, channel_name;
	iss >> target_nick >> channel_name; // Read target nickname and channel name
	// Simplified handling
	std::cout << "Client " << client_fd << " invited " << target_nick << " to channel " << channel_name << std::endl;
}

Client* Server::getClient(const int &fd) {
    // Iterate through the clients
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->first == fd) {
            return it->second; // Return the channel if found
        }
    }
    return NULL; // Use NULL in place of nullptr
}

Client* Server::getClient(const std::string& search) {
    // Iterate through the clients
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNick() == search) {
            return it->second; // Return the channel if found
        }
    }
	return NULL;
}

Client &Server::getClientRef(const int &fd) {
	Client *found = getClient(fd);
	if (!found)
		throw (std::runtime_error("Error: client not found: fd " + intToString(fd)));
	return (*getClient(fd));
}

Client &Server::getClientRef(const std::string &search) {
	Client *found = getClient(search);
	if (!found)
		throw (std::runtime_error("Error: client not found: " + search));
	return (*found);
}

Channel* Server::getChannel(const std::string& search) {
    // Iterate through the channels
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        if (it->first == search)
            return it->second; // Return the channel if found
    }
	return NULL;
}

Channel &Server::getChannelRef(const std::string &search) {
	Channel *found = getChannel(search);
	if (!found)
		throw (std::runtime_error("Error: channel not found: " + search));
	return (*found);
}
