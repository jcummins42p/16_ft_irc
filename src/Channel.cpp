/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:00:44 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/19 11:47:39 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

/* CONSTRUCT - DESTRUCT */

Channel::Channel(const std::string& in_name, const Client& creator)
	: hashed_pass(0), clnts_limit(MAX_CLIENTS), invite_only(false), topic_admins_only(false) {
	if (in_name.empty())
		name = "Default chat";
	else
		name = in_name;
	clients.insert(&creator);
	admins.insert(&creator);
}

Channel::~Channel(void) {}


/* PRIVATE METHODS */

ssize_t Channel::internalMessage(const Client& client, const std::string message) const {
	return (send(client.getFd(), message.c_str(), message.size(), 0));
}

/* GETTERS */

const std::string&	Channel::getName(void) const { return (name); }
const std::string&	Channel::getTopic(void) const { return (topic); }


/* SETTERS */

bool	Channel::setTopic(const std::string& in_topic, const Client& admin) {
	if (topic_admins_only && admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (in_topic.empty()) {
		internalMessage(admin, "Can't set an empty topic!\n");
		return (false);
	}
	else {
		topic = in_topic;
		return (true);
	}
}

bool	Channel::setPass(std::string& in_pass, const Client& admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (in_pass.empty()) {
		internalMessage(admin, "Can't set an empty password!\n");
		return (false);
	}
	else {
		hashed_pass = hashSimple(in_pass);
		return (true);
	}
}

/* ADMIN FUNCTIONS */

bool	Channel::addClient(const Client& in_client, const Client& admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (clients.size() >= clnts_limit) {
		internalMessage(admin, "Can't add a client - the channel is full!\n");
		return (false);
	}
	else {
		clients.insert(&in_client);
		return (true);
	}
}

bool	Channel::addAdmin(const Client& in_client, const Client& admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (clients.find(&in_client) == clients.end()) {
		internalMessage(admin, "Can't make an admin - the client is not in the channel!\n");
		return (false);
	}
	else {
		admins.insert(&in_client);
		return (true);
	}
}

bool	Channel::kickClient(const Client& in_client, const Client& admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (clients.find(&in_client) == clients.end()) {
		internalMessage(admin, "Can't kick - the user is not in the channel!\n");
		return (false);
	}
	else {
		clients.erase(&in_client);
		admins.erase(&in_client);
		return (true);
	}
}

bool	Channel::kickAdmin(const Client& in_client, const Client& admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (clients.find(&in_client) == clients.end()) {
		internalMessage(admin, "Can't kick - the user is not in the channel!\n");
		return (false);
	}
	else if (admins.find(&in_client) == admins.end()) {
		internalMessage(admin, "Can't kick - the user is not an admin!\n");
		return (false);
	}
	else {
		admins.erase(&in_client);
		return (true);
	}
}


/* JOIN - LEAVE */

bool	Channel::joinChannel(const Client& in_client) {
	if (invite_only) {
		internalMessage(in_client, "Sorry, the chat is invite-only!\n");
		return (false);
	}
	else if (clients.size() >= clnts_limit) {
		internalMessage(in_client, "Sorry, the chat is full!\n");
		return (false);
	}
	else {
		clients.insert(&in_client);
		return (true);
	}
}

bool	Channel::leaveChannel(const Client& in_client) {
	admins.erase(&in_client);
	if (admins.empty()) {
		if (!clients.empty())
			admins.insert(*clients.begin());
	}
	clients.erase(&in_client);
	return (true);
}

/* GROUP MESSAGES */

void	Channel::channelMessage(const std::string message, const Client& sender) const {
	for (std::set<const Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
		if (*it != &sender)
			send((*it)->getFd(), message.c_str(), message.size(), 0);
}
