/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCommands.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 15:21:19 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/13 18:45:53 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::handleNickCommand(int client_fd, std::istringstream& iss)
{
	std::string in_nick;
	iss >> in_nick;
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
	iss >> in_username;
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

	if (channelName.empty())
	   channelName = "#Default";
	Channel* channel = getChannel(channelName);
	if (channel == NULL) {	// If the channel doesn't exist, create it
		try { channel = createChannel(client_fd, channelName, password); }
		catch ( std::invalid_argument &e ) {
			log.error("Join: " + getClientRef(client_fd).getNick()
					+ " failed to create channel " + channelName + ": "
					+ std::string(e.what()));
			sendString(client_fd, e.what());
		}
	}
	else {	// Otherwise, join the existing channel
		try {
			channel->joinChannel(*clients[client_fd], password);
			log.info(getClientRef(client_fd).getNick()
					+ " joined existing channel " + channel->getName());
		} catch (std::exception &e) {
			log.error("Join: " + getClientRef(client_fd).getNick()
					+ " failed to join: " + std::string(e.what()));
			sendString(client_fd, "Failed to join: " + std::string(e.what()));
		}
	}
}

void Server::handlePartCommand(int client_fd, std::istringstream &iss)
{
	std::string channel_name;
	iss >> channel_name;

	try {
		Channel &channel = getChannelRef(channel_name);
		if (!getClientRef(client_fd).isInChannel(channel))
			throw (std::runtime_error("Not in channel " + channel_name));
		channel.channelMessage(channel_name + ": " + clients[client_fd]->getNick()
				+ " has left channel", *clients[client_fd]);
		channel.removeClient(*clients[client_fd]);
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
	Channel &channel = getChannelRef(target);
	std::string formattedMsg = channel.getName()
		+ " message from " + clients[client_fd]->getNick() + ": " + msg;
	channel.channelMessage(formattedMsg, *clients[client_fd]);
	return (0);
}

void Server::handlePrivmsgCommand(int client_fd, std::istringstream &iss) {
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

void Server::handleQuitCommand(int client_fd, std::istringstream &iss) {
	(void) iss;
	std::cout << "Client " << client_fd << " disconnected." << std::endl;
	close(client_fd); // Close the client's socket
	clients.erase(client_fd); // Remove client from the map
}

void Server::handleTopicCommand(int client_fd, std::istringstream &iss) {
	std::string channel_name, new_topic;
	iss >> channel_name; // read channel name
	std::getline(iss, new_topic); // read the new topic

	try {
		colonectomy(new_topic);
		Channel &channel = getChannelRef(channel_name);
		channel.setTopic(new_topic, *clients[client_fd]);
		sendString(client_fd, channel_name
				+ " topic changed to '" + new_topic + '"');
	} catch (std::exception &e) {
		log.error("Topic: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
}

void Server::handleModeCommand(int client_fd, std::istringstream &iss) {
	std::string channel_name, mode, message;
	iss >> channel_name; // read channel name and mode

	try {
		Channel &channel = getChannelRef(channel_name);
		message = channel.modeHandler(client_fd, iss);
		log.info("Mode: " + message);
		sendString(client_fd, message);
	}
	catch (std::exception &e) {
		log.error("Mode: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
}

void Server::handleKickCommand(int client_fd, std::istringstream &iss) {
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

void Server::handleBanCommand(int client_fd, std::istringstream &iss) {
	std::string channel_name, target;
	iss >> channel_name >> target; // read channel name and target nickname

	log.info("Client " + intToString(client_fd) + " attempting to ban "
			+ target + " from channel " + channel_name);
	try {
		getChannelRef(channel_name).banClient(getClientRef(target), getClientRef(client_fd));
		sendString(client_fd, "Successfully banned " + target
				+ " from channel " + channel_name);
	}
	catch ( std::runtime_error &e ) {
		log.error("Ban: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
}

void Server::handleInviteCommand(int client_fd, std::istringstream &iss) {
	std::string target_nick, channel_name;
	iss >> target_nick >> channel_name;
	try {
		getChannelRef(channel_name).inviteClient(getClientRef(target_nick), getClientRef(client_fd));
	} catch (std::exception &e) {
		log.error("Invite: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
}

void Server::handleListCommand(int client_fd, std::istringstream &iss) {
	(void) iss;
	std::string message = "";
	int	listlen = 0;

	try
	{
		if (channels.empty())
			throw std::runtime_error("No active channels on this server!");
		for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			Channel *channel = it->second;
			if (!channel->isSecret() || channel->containsMember(getClientRef(client_fd))) {
				if (listlen++ > 0)
					message += "\n";
				message += channel->getName();
				if (channel->hasTopic())
					message += ": Topic: " + channel->getTopic();
			}
		}
		sendString(client_fd, message);
	}
	catch (std::runtime_error &e) {
		log.error("List: " + std::string(e.what()));
		sendString(client_fd, std::string(e.what()));
	}
}

void Server::handleDieCommand(int client_fd, std::istringstream &iss) {
	(void) iss;
	static int timescalled = 0;
	broadcastMessage("Server shutting down, please close your session");
	// broadcast message isn't working, it seems. Perhaps it needs more cycles
	// to actually send what this puts in the outbuffer
	// likely we want to set a time limit in the main loop to live for, while sending messages
	(void) client_fd;
	if (timescalled++ == 1)
		_running = false;
	// obviously this needs more checks, and if the fd 5 user leaves then it cannot be shut
	// but is this reallt a problem? It's just there to demonstrate the destructor.
}
