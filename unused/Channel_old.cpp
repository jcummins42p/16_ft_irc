/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_old.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/19 11:44:57 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(const std::string& name) :
	name(name),
	invite_only(false),
	topic_restricted(false),
	user_limit(-1) {}

Channel::~Channel( void )
{
	std::cout << "Reminder that you need to implement channel destructor" << std::endl;
}

void Channel::join(int client_fd)
{
	clients.insert(client_fd);
}

void Channel::part(int client_fd)
{
	clients.erase(client_fd);
}

void Channel::channelMessage(const std::string& message, int sender_fd)
{
	//for loop to iterate through the set of clients
	for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) {
		int client_fd = *it;
		if (client_fd != sender_fd) {
			send(client_fd, message.c_str(), message.length(), 0);
		}
	}
}

void Channel::setTopic(const std::string& new_topic)
{
	topic = new_topic;
}

std::string Channel::getTopic() const
{
	return topic;
}

void Channel::setPassword(const std::string& pass)
{
	password = pass;
}

bool Channel::isOperator(int client_fd)
{
	return clients.find(client_fd) != clients.end(); // Simplified for demonstration
}
