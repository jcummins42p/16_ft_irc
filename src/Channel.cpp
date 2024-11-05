/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:00:44 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/05 08:41:46 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(const std::string& in_name, Client& creator)
	: clnts_limit(MAX_CLIENTS), invite_only(false), topic_admins_only(false) {
	if (in_name.empty())
		name = "Default chat";
	else
		name = in_name;
	clients.push_back(&creator);
	admins.push_back(&creator);
}

Channel::~Channel(void) {}


/* GETTERS */

std::string	Channel::getName(void) const {
	return (name);
}

std::string	Channel::getTopic(void) const {
	return (topic);
}


/* SETTERS */

bool	Channel::setTopic(const std::string& in_topic, const Client& modifier) {
	if (isAdmin(modifier)) {
		topic = in_topic;
		return (true);
	}
	else {
		std::cerr << "This user doesn't have admin rights!" << std::endl;
		return (false);
	}
}

bool	Channel::setKey(const std::string& in_key, const Client& modifier) {
	if (isAdmin(modifier)) {
		key = hashingFunc(in_key);
		return (true);
	}
	else {
		std::cerr << "This user doesn't have admin rights!" << std::endl;
		return (false);
	}
}


