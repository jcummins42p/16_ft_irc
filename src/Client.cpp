/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/09 17:50:29 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


// Construct - destruct

Client::Client(int fd, Server &server) :
	fd(fd),
	is_authenticated(false),
	server(server)
{}

Client::~Client( void ) {
	std::cout << "Reminder that you need to implement Client destructor" << std::endl;
}

// Getter - setters
const int&			Client::getFd(void) const { return (fd); }
const std::string&	Client::getNick(void) const { return nick; }
const std::string&	Client::getUser(void) const { return user; }

void	Client::setAuthenticated(void) { is_authenticated = true; }
bool	Client::isAuthenticated(void) const { return is_authenticated; }

void	Client::setRegistered(void) { is_registered = true; }
bool	Client::isRegistered(void) const { return (is_registered); }

static void validateUser( const std::string &name ) {
	if (name.empty())
		throw std::invalid_argument("Can't set an empty username!");
	for (unsigned long i = 0; i < name.size(); i++) {
		if (name[i] == ',')
			throw std::invalid_argument("User name must not contain ','");
		if (name[i] == ' ')
			throw std::invalid_argument("User name must not contain ' '");
	}
	if (caseInsCompare(name, "admin") || caseInsCompare(name, "root"))
		throw std::invalid_argument("Inappropriate username");
}

static void validateNick( const std::string &nick ) {
	if (nick.empty())
		throw std::invalid_argument("Can't set an empty nick");
	if (!isalpha(nick[0]))
		throw std::invalid_argument("Nick must begin with a letter");
	for (unsigned long i = 0; i < nick.size(); i++) {
		if (nick[i] == ',')
			throw std::invalid_argument("Nick name must not contain ','");
		if (nick[i] == ' ')
			throw std::invalid_argument("Nick name must not contain ' '");
	}
	if (caseInsCompare(nick, "admin") || caseInsCompare(nick, "root")) // if same, returns true
		throw std::invalid_argument("Inappropriate nickname");
}

void	Client::setNick(const std::string& in_nick) {
	validateNick(in_nick);
	if (!getUser().empty())
		setRegistered();
	nick = in_nick;
}

void	Client::setUser(const std::string& in_username) {
	validateUser(in_username);
	if (!getNick().empty())
		setRegistered();
	user = in_username;
}

const Server &Client::getServer( void ) {
	return this->server;
}

bool	Client::isInChannel( const Channel &channel ) const {
	return (channel.containsMember(*this));
}

bool	Client::isInChannel( const Channel *channel ) const {
	if (!channel)
		return (false);
	return (channel->containsMember(*this));
}
