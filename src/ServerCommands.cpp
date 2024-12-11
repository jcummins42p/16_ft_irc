/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCommands.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 15:21:19 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/11 19:35:49 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handleNickCommand(int client_fd, std::istringstream& iss)
{
	std::string in_nick;
	iss >> in_nick; // Read the nickname from the stream
	try {
		clients[client_fd]->setNick(in_nick);
		log.info("Client " + intToString(client_fd)
				+ " set nickname to " + clients[client_fd]->getNick());
		if (in_nick.size() > clients[client_fd]->getNick().size())
			sendString(client_fd, "Nickname too long, truncating to fit "
					+ intToString(NICK_MAX_LEN) + " char limit");
	}
	catch ( std::exception &e ) {
		log.warn("Nick: Client " + intToString(client_fd)
				+ ": " + std::string(e.what()));
		sendString(client_fd, e.what());
	}
}

void Server::handleUserCommand(int client_fd, std::istringstream& iss)
{
	std::string in_username;
	iss >> in_username; // Read the username from the stream
	try {
		clients[client_fd]->setUser(in_username);
		log.info("Client " + intToString(client_fd)
				+ " set username to " + clients[client_fd]->getUser());
		if (in_username.size() > clients[client_fd]->getUser().size())
			sendString(client_fd, "Username too long, truncating to fit "
					+ intToString(USER_MAX_LEN) + " char limit");
	}
	catch ( std::exception &e ) {
		sendString(client_fd, e.what());
		log.warn("User: Client " + intToString(client_fd)
				+ ":" + std::string(e.what()));
	}
}

void Server::handleJoinCommand(int client_fd, std::istringstream& iss) {
	std::string channelName;
	std::string password;
	iss >> channelName >> password;

	// Look for the channel by name
	// Now that there is exception thrown in getChannel, we have problems
	// Maybe move check to the getChannelRef instead
	Channel* channel = getChannel(channelName);
	// If the channel doesn't exist, create it
	if (channel == NULL) {
		try { channel = createChannel(client_fd, channelName, password); }
		catch ( std::invalid_argument &e ) {
			log.error("Join: " + getClientRef(client_fd).getNick()
					+ " failed to create channel " + channelName + ": "
					+ std::string(e.what()));
			sendString(client_fd, e.what());
		}
	}
	else {
		// Otherwise, join the existing channel
		try {
			channel->joinChannel(*clients[client_fd], password);
			log.info(getClientRef(client_fd).getNick()
					+ " joined existing channel " + channel->getName());
			sendString(client_fd, "Joined channel " + channel->getName());
			if (channel->hasTopic())
				sendString(client_fd, channel->getName() + " topic: " + channel->getTopic());
		} catch (std::exception &e) {
			log.error("Join: " + getClientRef(client_fd).getNick()
					+ " failed to join: " + std::string(e.what()));
			sendString(client_fd, "Failed to join: " + std::string(e.what()));
		}
	}
}

void Server::handlePartCommand(int client_fd, std::istringstream& iss)
{
	std::string channel_name;
	iss >> channel_name;

	try {
		Channel &channel = getChannelRef(channel_name);
		if (!getClientRef(client_fd).isInChannel(channel))
			throw (std::runtime_error("Not in channel " + channel_name));
		channel.removeClient(*clients[client_fd]);
		sendString(client_fd, channel_name + ": you left channel" + channel_name );
		channel.channelMessage(channel_name + ": " + clients[client_fd]->getNick()
				+ " has left channel", *clients[client_fd]);
	}
	catch (std::runtime_error &e) {
		log.error("Part: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
	catch (std::exception &e) {
		log.error("Closing empty channel " + channel_name);
		sendString(client_fd, "Closing empty channel " + channel_name);
		channels.erase(channel_name);
	}
}

bool Server::sendMsgToChannel(int client_fd, const std::string &target, const std::string &msg) {
	if (target[0] != '#' && target[0] != '&')
		return (1);
	// Look for the channel by name
	Channel* channel = getChannel(target);
	if (!getClientRef(client_fd).isInChannel(channel))
		throw (std::runtime_error("Not in channel " + target));
	std::string formattedMsg = channel->getName()
		+ " message from " + clients[client_fd]->getNick() + ": " + msg;
	channel->channelMessage(formattedMsg, *clients[client_fd]);
	return (0);
}

static void colonectomy( std::string &msg ) {
	if (!msg.empty()) {
		size_t colpos = msg.find(":", 0);
		if (colpos != std::string::npos && colpos + 1 < msg.size())
			msg = msg.substr(colpos + 1);
		else
			throw std::runtime_error("Message must begin with :");
	}
}

void Server::handlePrivmsgCommand(int client_fd, std::istringstream& iss) {
	std::string target;
	std::string msg;
	iss >> target;
	std::getline(iss, msg);

	try {
		colonectomy(msg);
		if (!sendMsgToChannel(client_fd, target, msg))
			return ;
		// If the target is a client (not a channel), find the client by nickname
		std::string formattedMsg = "Private message from "
			+ clients[client_fd]->getNick() + ": " + msg;
		return (sendString(getClientFd(target), formattedMsg));
	}
	// If the target client was not found
	catch ( std::exception &e ) {
		log.error("Privmsg: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
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

	try {
		colonectomy(new_topic);
		Channel &channel = getChannelRef(channel_name);
		channel.setTopic(new_topic, *clients[client_fd]);
		sendString(client_fd, channel_name
				+ " topic changed to '" + new_topic + '"');
		channel.channelMessage(channel_name
				+ " topic changed by " + getClientRef(client_fd).getNick()
				+ " to '" + new_topic + "'", *clients[client_fd]);
	} catch (std::exception &e) {
		log.error("Topic: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
}

void Server::handleModeCommand(int client_fd, std::istringstream& iss)
{
	std::string channel_name, mode, input, message;
	bool toggle = false;
	iss >> channel_name >> mode; // read channel name and mode

	try {
		if (mode.size() != 2)
			throw std::runtime_error("Invalid mode switch length: " + intToString(mode.size()));
		if (mode[0] == '+')
			toggle = true;
		else if (mode[0] != '-')
			throw std::runtime_error("Mode switch must start with '-' or '+");
		std::getline(iss, input);
		colonectomy(input);
		if (mode[1] == 'i')
			message = getChannelRef(channel_name).handleModeInvite(client_fd, input, toggle);
		else if (mode[1] == 't')
			message = getChannelRef(channel_name).handleModeTopic(client_fd, input, toggle);
		else if (mode[1] == 'k')
			message = getChannelRef(channel_name).handleModeKey(client_fd, input, toggle);
		else if (mode[1] == 'o')
			message = getChannelRef(channel_name).handleModeOperator(client_fd, input, toggle);
		else if (mode[1] == 'l')
			message = getChannelRef(channel_name).handleModeUserLimit(client_fd, input, toggle);
		else
			throw std::runtime_error("Invalid mode '" + mode + "'");
		log.info("Mode: " + message);
		sendString(client_fd, message);
	}
	catch (std::exception &e) {
		log.error("Mode: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
}

void Server::handleKickCommand(int client_fd, std::istringstream& iss) {
	std::string channel_name, target;
	iss >> channel_name >> target; // read channel name and target nickname

	log.info("Client " + intToString(client_fd) + " attempting to kick "
			+ target + " from channel " + channel_name);
	try {
		getChannelRef(channel_name).kickClient(getClientRef(target), getClientRef(client_fd));
		sendString(client_fd, "Successfully kicked " + target
				+ " from channel " + channel_name);
	}
	catch ( std::runtime_error &e ) {
		log.error("Kick: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
}

void Server::handleInviteCommand(int client_fd, std::istringstream& iss) {
	std::string target_nick, channel_name;
	iss >> target_nick >> channel_name; // Read target nickname and channel name
	// Simplified handling
	std::cout << "Client " << client_fd << " invited " << target_nick << " to channel " << channel_name << std::endl;
}
