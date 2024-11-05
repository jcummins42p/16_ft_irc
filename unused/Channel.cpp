/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:00:44 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/05 15:27:45 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

/* CONSTRUCT - DESTRUCT */

Channel::Channel(const std::string& in_name, const Client& creator)
	: clnts_limit(MAX_CLIENTS), invite_only(false), topic_admins_only(false) {
	if (in_name.empty())
		name = "Default chat";
	else
		name = in_name;
	clients.push_back(&creator);
	admins.push_back(&creator);
}

Channel::~Channel(void) {}


/* PRIVATE METHODS */

int	Channel::posInList(const std::vector<const Client*>& list, const Client& in_client) const {
	for (size_t i = 0; i < list.size(); i++)
		if (in_client.getSocket() == list[i]->getSocket())
			return (i);
	return (CHANNEL_ERROR);
}

// Maybe we should send errors to server somehow instead of outputs like cout or cerr?
bool Channel::notAnAdmin(void) const {
	std::cerr << "You don't have admin rights!" << std::endl;
	return (false);
}


/* GETTERS */

std::string	Channel::getName(void) const {
	return (name);
}

std::string	Channel::getTopic(void) const {
	return (topic);
}


/* SETTERS */

bool	Channel::setTopic(const std::string& in_topic, const Client& admin) {
	if (topic_admins_only && posInList(admins, admin) == CHANNEL_ERROR)
		return (notAnAdmin());
	if (in_topic.empty()) {
		std::cerr << "Can't add an empty topic!" << std::endl;
		return (false);
	}
	else {
		topic = in_topic;
		return (true);
	}
}

bool	Channel::setKey(const std::string& in_key, const Client& admin) {
	if (posInList(admins, admin) == CHANNEL_ERROR)
		return (notAnAdmin());
	if (in_key.empty()) {
		std::cerr << "Can't add an empty key" << std::endl;
		return (false);
	}
	else {
		key = hashingFunc(in_key);
		return (true);
	}
}

/* ADMIN FUNCTIONS */

bool	Channel::addClient(const Client& in_client, const Client& admin) {
	if (posInList(admins, admin) == CHANNEL_ERROR)
		return (notAnAdmin());
	if (posInList(clients, in_client) != CHANNEL_ERROR) {
		std::cerr << "The client is already in the channel!" << std::endl;
		return (false);
	}
	else {
		clients.push_back(&in_client);
		return (true);
	}
}

bool	Channel::addAdmin(const Client& in_client, const Client& admin) {
	if (posInList(admins, admin) == CHANNEL_ERROR)
		return (notAnAdmin());
	else if (posInList(clients, in_client) == CHANNEL_ERROR) {
		std::cerr << "The client is not in the channel!" << std::endl;
		return (false);
	}
	else if (posInList(admins, in_client) != CHANNEL_ERROR) {
		std::cerr << "The client is already an admin!" << std::endl;
		return (false);
	}
	else {
		admins.push_back(&in_client);
		return (true);
	}
}

bool	Channel::kickClient(const Client& in_client, const Client& admin) {
	int	pos;

	if (posInList(admins, admin) == CHANNEL_ERROR)
		return (notAnAdmin());
	if (posInList(clients, in_client) == CHANNEL_ERROR) {
		std::cerr << "The client is not in the channel!" << std::endl;
		return (false);
	}

	pos = posInList(clients, in_client);
	if (pos == CHANNEL_ERROR) {
		std::cerr << "The client is not in the channel!" << std::endl;
		return (false);
	}
	clients.erase(clients.begin() + pos);

	pos = posInList(admins, in_client);
	if (pos != CHANNEL_ERROR)
		clients.erase(admins.begin() + pos);
	return (true);
}

bool	Channel::kickAdmin(const Client& in_client, const Client& admin) {
	int	pos;

	if (posInList(admins, admin) == CHANNEL_ERROR)
		return (notAnAdmin());

	pos = posInList(admins, in_client);
	if (pos == CHANNEL_ERROR) {
		std::cerr << "The client is not an admin!" << std::endl;
		return (false);
	}
	else {
		admins.erase(admins.begin() + pos);
		return (true);
	}
}


/* JOIN - LEAVE */


