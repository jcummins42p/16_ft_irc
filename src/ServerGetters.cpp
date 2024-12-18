/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerGetters.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 15:23:17 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/18 21:48:42 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

const int &Server::getClientFd(const std::string &search) const {
	return (getClientRef(search).getFd());
}

std::string Server::serverName( void ) { return ( _name ); }

int	Server::getFd( void ) { return (server_fd); }

Client* Server::getClient(const int &fd) const {
	// Iterate through the clients
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->first == fd) {
			return it->second; // Return the channel if found
		}
	}
	return NULL; // Use NULL in place of nullptr
}

Client* Server::getClient(const std::string& search) const {
	// Iterate through the clients
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->getNick() == search) {
			return it->second; // Return the channel if found
		}
	}
	return NULL;
}

Client &Server::getClientRef(const int &fd) const {
	Client *found = getClient(fd);
	if (!found)
		throw (std::runtime_error("Client not found: fd " + intToString(fd)));
	return (*getClient(fd));
}

Client &Server::getClientRef(const std::string &search) const {
	Client *found = getClient(search);
	if (!found)
		throw (std::runtime_error("Client not found: " + search));
	return (*found);
}

Channel* Server::getChannel(const std::string& search) const {
	// Iterate through the channels
	for (std::map<std::string, Channel*>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
		if (it->first == search)
			return it->second; // Return the channel if found
	}
	return NULL;
}

Channel &Server::getChannelRef(const std::string &search) const {
	Channel *found = getChannel(search);
	if (!found)
		throw (std::runtime_error("Channel not found: " + search));
	return (*found);
}
