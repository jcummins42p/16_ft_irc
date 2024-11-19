/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRCServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/19 13:28:12 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include "IRCServer.hpp"

IRCServer::IRCServer(int port, std::string in_pass)
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	hashed_pass = hashSimple(in_pass);

	bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	listen(server_fd, 5);
}

IRCServer::~IRCServer( void )
{
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		close(it->first);
		delete it->second;
	}

	for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		delete it->second;
	}
	close(server_fd);
}

void IRCServer::run()
{
	struct pollfd fds[MAX_CLIENTS + 1];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;

	for (int i = 1; i <= MAX_CLIENTS; i++) {
		fds[i].fd = -1;
	}

	while (true) {
		int ret = poll(fds, MAX_CLIENTS + 1, -1);
		(void)ret;
		poll(fds, MAX_CLIENTS + 1, -1);
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

//	private //
void IRCServer::acceptClient(struct pollfd* fds)
{
	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd < 0)
	{
		std::cerr << "Failed to accept client" << std::endl;
		return;
	}

	std::cout << "Client connected: " << client_fd << std::endl;
	clients[client_fd] = new Client(client_fd);
	const std::string prompt = "Please enter the password: ";
    send(client_fd, prompt.c_str(), prompt.size(), 0);

	// add new client to poll array
	for (int i = 1; i <= MAX_CLIENTS; i++) {
		if (fds[i].fd == -1) {
			fds[i].fd = client_fd;
			fds[i].events = POLLIN;
			break;
		}
	}
}

void IRCServer::handleClient(int client_fd)
{
	char buffer[BUFFER_SIZE];
	ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received <= 0) {
		if (bytes_received == 0) {
			std::cout << "Client disconnected: " << client_fd << std::endl;
		} else {
			std::cerr << "Receive error on client: " << client_fd << std::endl;
		}
		close(client_fd);
		clients.erase(client_fd);
		return;
	}

	buffer[bytes_received] = '\0';
	std::string message(buffer);

	if (!clients[client_fd]->getAuthentificated()) {
		if (!message.empty() && message[message.length() - 1] == '\n')
			message.erase(message.length() - 1);
		unsigned int in_hashed_pass = hashSimple(message);

		if (in_hashed_pass != this->hashed_pass) {
			const std::string prompt = "Wrong password, try again: ";
			send(client_fd, prompt.c_str(), prompt.size(), 0);
		}
		else {
			clients[client_fd]->setAuthentificated();
			const std::string prompt = "Autentification successful!\n";
			send(client_fd, prompt.c_str(), prompt.size(), 0);
		}
	}
	else
		processMessage(client_fd, message);
}

void IRCServer::processMessage(int client_fd, const std::string& message)
{
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
	} else
		std::cout << "Client number " << client_fd << ": " << message;
}

void IRCServer::handleNickCommand(int client_fd, std::istringstream& iss)
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

void IRCServer::handleUserCommand(int client_fd, std::istringstream& iss)
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

void IRCServer::handleJoinCommand(int client_fd, std::istringstream& iss)
{
	std::string channel_name;
	iss >> channel_name; // read the channel name from the stream
	Channel* channel;

	if (channel_name.empty()) {
		const std::string prompt = "Can't set an empty channel name!\n";
		send(client_fd, prompt.c_str(), prompt.size(), 0);
		return ;
	}
	// check if the channel exists
	if (channels.find(channel_name) == channels.end()) {
		channel = new Channel(channel_name, *clients[client_fd]); // create a new channel if it doesn't exist
		channels[channel_name] = channel; // add channel to the map
	} else {
		channel = channels[channel_name]; // get existing channel
	}

	channel->joinChannel(*clients[client_fd]); // add client to the channel
	clients[client_fd]->channels.insert(channel_name); // update clients channel list
	std::cout << "Client " << client_fd << " joined channel " << channel_name << std::endl;
}

void IRCServer::handlePartCommand(int client_fd, std::istringstream& iss)
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
void IRCServer::handlePrivmsgCommand(int client_fd, std::istringstream& iss)
{
	std::string target;
	std::string msg;
	iss >> target;
	std::getline(iss, msg);

	if (channels.find(target) != channels.end()) {
		channels[target]->channelMessage(clients[client_fd]->getNick() + ": " + msg + "\n", *clients[client_fd]);
	} else {
		send(client_fd, "Target not found.\n", 18, 0);
	}
}

void IRCServer::handleQuitCommand(int client_fd)
{
	std::cout << "Client " << client_fd << " disconnected." << std::endl;
	close(client_fd); // Close the client's socket
	clients.erase(client_fd); // Remove client from the map
}

void IRCServer::handleTopicCommand(int client_fd, std::istringstream& iss)
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

void IRCServer::handleModeCommand(int client_fd, std::istringstream& iss)
{
	std::string channel_name, mode;
	iss >> channel_name >> mode; // read channel name and mode
	//setting modes
	std::cout << "Client " << client_fd << " set mode for channel " << channel_name << " to " << mode << std::endl;
}

void IRCServer::handleKickCommand(int client_fd, std::istringstream& iss) {
	std::string channel_name, target_nick;
	iss >> channel_name >> target_nick; // read channel name and target nickname
	//
	std::cout << "Client " << client_fd << " kicked " << target_nick << " from channel " << channel_name << std::endl;
}

void IRCServer::handleInviteCommand(int client_fd, std::istringstream& iss) {
	std::string target_nick, channel_name;
	iss >> target_nick >> channel_name; // Read target nickname and channel name
	// Simplified handling
	std::cout << "Client " << client_fd << " invited " << target_nick << " to channel " << channel_name << std::endl;
}
