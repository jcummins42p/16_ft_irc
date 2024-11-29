/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/29 18:22:00 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <arpa/inet.h>
#include <ctime> // For time functions

Server::Server(int port, const std::string& in_pass) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    hashed_pass = hashSimple(in_pass);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    // Open log file
    logFile.open("server.log", std::ios::app);
    logEvent("INFO", "Server initialized on port " + intToString(port));
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

    logEvent("INFO", "Server shutting down.");
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Server::run() {
    struct pollfd fds[MAX_CLIENTS + 1];
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
    }

    logEvent("INFO", "Server is running. Waiting for connections...");
    while (true) {
        int ret = poll(fds, MAX_CLIENTS + 1, -1);
        (void)ret; // To suppress unused variable warning
        for (int i = 0; i <= MAX_CLIENTS; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) {
                    acceptClient(fds);
                } else {
                    handleClient(fds[i].fd);
                }
            }
        }
    }
}

void Server::acceptClient(struct pollfd* fds) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        logEvent("ERROR", "Failed to accept client.");
        return;
    }

    logEvent("INFO", "Client connected: FD " + intToString(client_fd));
    clients[client_fd] = new Client(client_fd);
    const std::string prompt = "Please enter the password: ";
    send(client_fd, prompt.c_str(), prompt.size(), 0);

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
            logEvent("INFO", "Client disconnected: FD " + intToString(client_fd));
        } else {
            logEvent("ERROR", "Receive error on client: FD " + intToString(client_fd));
        }
        close(client_fd);
        clients.erase(client_fd);
        return;
    }

    buffer[bytes_received] = '\0';
    std::string message(buffer);
    logEvent("INFO", "Received message from FD " + intToString(client_fd) + ": " + message);

    if (!clients[client_fd]->getAuthentificated()) {
        if (!message.empty() && message[message.length() - 1] == '\n')
            message.erase(message.length() - 1);
        unsigned int in_hashed_pass = hashSimple(message);

        if (in_hashed_pass != this->hashed_pass) {
            const std::string prompt = "Wrong password, try again: ";
            send(client_fd, prompt.c_str(), prompt.size(), 0);
            logEvent("WARNING", "Authentication failed for FD " + intToString(client_fd));
        } else {
            clients[client_fd]->setAuthentificated();
            const std::string prompt = "Authentication successful!\n";
            send(client_fd, prompt.c_str(), prompt.size(), 0);
            logEvent("INFO", "Client authenticated: FD " + intToString(client_fd));
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
        logEvent("INFO", "Unhandled message from FD " + intToString(client_fd) + ": " + message);
    }
}

void Server::handleNickCommand(int client_fd, std::istringstream& iss)
{
	std::string in_nick;
	iss >> in_nick; // Read the nickname from the stream
	if (in_nick.empty()) {
		const std::string prompt = "You can't set an empty nickname!\n";
		send(client_fd, prompt.c_str(), prompt.size(), 0);
	}
	else {
		clients[client_fd]->setNick(in_nick); // Set the client's nickname
		std::cout << "Client " << client_fd << " set nickname to " << clients[client_fd]->getNick() << std::endl;
	}
}

void Server::handleUserCommand(int client_fd, std::istringstream& iss)
{
	std::string in_username;
	iss >> in_username; // Read the username from the stream
	if (in_username.empty()) {
		const std::string prompt = "You can't set an empty username!\n";
		send(client_fd, prompt.c_str(), prompt.size(), 0);
	}
	else {
		clients[client_fd]->setUser(in_username); // Set the client's username
		std::cout << "Client " << client_fd << " set username to " << clients[client_fd]->getUser() << std::endl;
	}
}

void Server::handleJoinCommand(int client_fd, std::istringstream& iss) {
    std::string channelName;
    std::string password;
    iss >> channelName >> password;

    // Look for the channel by name
    Channel* channel = getChannelByName(channelName.substr(1));  // Remove the '#' from channel name
    if (channel == NULL) {
        // If the channel doesn't exist, create it
        channel = new Channel(channelName.substr(1), *clients[client_fd], password);  // Assuming the client is the creator
        channels[channelName.substr(1)] = channel;  // Add the new channel to the map
        send(client_fd, "Joined channel successfully.\n", 29, 0);
    } else {
        // Otherwise, join the existing channel
        if (channel->joinChannel(*clients[client_fd], password)) {
            send(client_fd, "Joined channel successfully.\n", 29, 0);
        } else {
            send(client_fd, "Failed to join channel.\n", 24, 0);
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
		send(client_fd, ("Left channel: " + channel_name + "\n").c_str(), 25, 0);
		channels[channel_name]->channelMessage(clients[client_fd]->getNick() + " has left the channel.\n", *clients[client_fd]);
	} else {
		send(client_fd, "Channel not found.\n", 20, 0);
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
        // Remove the '#' from the channel name
        std::string channelName = target.substr(1);

        // Look for the channel by name
        Channel* channel = getChannelByName(channelName);
        if (channel) {
            // Send the message to all clients in the channel
            std::string formattedMsg = "Message from " + clients[client_fd]->getNick() + ": " + msg + "\n";
            channel->channelMessage(formattedMsg, *clients[client_fd]);
            return;
        } else {
            // If the channel is not found
            send(client_fd, "Channel not found.\n", 19, 0);
            return;
        }
    }

    // If the target is a client (not a channel), find the client by nickname
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second->getNick() == target) {
            // Send the private message to the target client
            std::string formattedMsg = "Private message from " + clients[client_fd]->getNick() + ": " + msg + "\n";
            send(it->first, formattedMsg.c_str(), formattedMsg.size(), 0);
            return;
        }
    }

    // If the target client was not found
    send(client_fd, "Target client not found.\n", 26, 0);
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

	if (new_topic.empty()) {
		const std::string prompt = "Cannot set an empty topic!\n";
		send(client_fd, prompt.c_str(), prompt.size(), 0);
		return ;
	}

	if (channels.find(channel_name) != channels.end()) {
		Channel* channel = channels[channel_name]; // get channel
		channel->setTopic(new_topic, *clients[client_fd]); // set the topic
		channel->channelMessage("TOPIC " + channel_name + " : " + new_topic + "\n", *clients[client_fd]); // notify clients of the new topic
	} else {
		send(client_fd, "No such channel.\n", 18, 0); // notify client of error
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
	std::string channel_name, target_nick;
	iss >> channel_name >> target_nick; // read channel name and target nickname
	//
	std::cout << "Client " << client_fd << " kicked " << target_nick << " from channel " << channel_name << std::endl;
}

void Server::handleInviteCommand(int client_fd, std::istringstream& iss) {
	std::string target_nick, channel_name;
	iss >> target_nick >> channel_name; // Read target nickname and channel name
	// Simplified handling
	std::cout << "Client " << client_fd << " invited " << target_nick << " to channel " << channel_name << std::endl;
}

// Centralized logging function
void Server::logEvent(const std::string& level, const std::string& message) {
    time_t now = time(0);
    struct tm* localTime = localtime(&now);

    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localTime);

    std::string logMessage = "[" + std::string(buf) + "] [" + level + "] " + message;

    // Print to terminal
    std::cout << logMessage << std::endl;

    // Write to log file
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
    }
}

std::string Server::intToString(int number) {
    std::ostringstream oss;
    oss << number;
    return oss.str();
}

Channel* Server::getChannelByName(const std::string& channelName) {
    // Iterate through the channels
    for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
        if (it->first == channelName) {
            return it->second; // Return the channel if found
        }
    }
    return NULL; // Use NULL in place of nullptr
}
