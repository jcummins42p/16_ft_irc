/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCommands.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 15:21:19 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/09 18:48:10 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handleNickCommand(int client_fd, std::istringstream& iss)
{
	std::string in_nick;
	iss >> in_nick; // Read the nickname from the stream
	try {
		clients[client_fd]->setNick(in_nick);
		log.info("Client " + intToString(client_fd) + " set nickname to " + clients[client_fd]->getNick());
		if (in_nick.size() > clients[client_fd]->getNick().size())
			sendString(client_fd, "Nickname too long, truncating to fit " + intToString(NICK_MAX_LEN) + " char limit");
		sendString(client_fd, "Successfully set nickname to " + clients[client_fd]->getNick() );
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
		if (in_username.size() > clients[client_fd]->getUser().size())
			sendString(client_fd, "Username too long, truncating to fit " + intToString(USER_MAX_LEN) + " char limit");
		sendString(client_fd, "Successfully set username to " + clients[client_fd]->getUser());
	}
	catch ( std::invalid_argument &e ) {
		log.warn("Client " + intToString(client_fd) + " failed to set empty username");
		sendString(client_fd, e.what());
	}
	catch ( std::exception &e ) {
		std::cerr << e.what() << std::endl;
	}
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
		//clients[client_fd]->channels.erase(channel_name);
		//clients[client_fd]->leaveChannel(channel_name);
		sendString(client_fd, "Left channel: " + channel_name );
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
		if (!getClientRef(client_fd).isInChannel(channel))
			return (sendString(client_fd, "Not in channel " + target));
		std::string formattedMsg = "Message from " + clients[client_fd]->getNick() + ": " + msg;
		channel->channelMessage(formattedMsg, *clients[client_fd]);
		return;
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
