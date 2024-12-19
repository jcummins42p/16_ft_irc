/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerGetters.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.com>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 15:23:17 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/19 23:03:17 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

const int &Server::getClientFd(const std::string &search) const {
	return (getClientRef(search).getFd());
}

std::string Server::serverName( void ) { return ( _name ); }

int	Server::getFd( void ) { return (server_fd); }

Client* Server::getClient(const int &fd) const {
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->first == fd) {
			return it->second;
		}
	}
	return NULL;
}

Client* Server::getClient(const std::string& search) const {
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->getNick() == search) {
			return it->second;
		}
	}
	return NULL;
}

Client &Server::getClientRef(const int &fd) const {
	Client *found = getClient(fd);

	if (found == NULL)
		throw (std::runtime_error("Server failed to find Client, internal error!"));
	return (*found);
}

Client &Server::getClientRef(const std::string &search) const {
	Client *found = getClient(search);

	if (found == NULL)
		throw (std::runtime_error("Server failed to find Client, internal error!"));
	return (*found);
}

Channel* Server::getChannel(const std::string& search) const {
	for (std::map<std::string, Channel*>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
		if (it->first == search)
			return it->second;
	}
	return NULL;
}

Channel &Server::getChannelRef(const int &exec_fd, const std::string &search) const {
	Channel *found = getChannel(search);
	Client *executor = getClient(exec_fd);

	if (executor == NULL)
		throw (std::runtime_error("Server failed to find Channel, internal error!"));
	if (!found)
		throw (std::runtime_error("403 " + executor->getNick() + " " + search + " :No such channel"));
	return (*found);
}

Channel &Server::getChannelRef(const std::string &search) const {
	Channel *found = getChannel(search);

	if (!found)
		throw (std::runtime_error("Server failed to find Channel, internal error!"));
	return (*found);
}
