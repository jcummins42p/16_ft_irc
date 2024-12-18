/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/18 21:39:21 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

const static std::string allowedchars = "`|^_-{}[]\\";

// Construct - destruct

Client::Client(int fd, Server &server) :
	fd(fd),
	is_authenticated(false),
	is_registered(false),
	server(server)
{}

Client::~Client( void ) {
}

// Getter - setters
const int&			Client::getFd(void) const { return (fd); }

std::string Client::getNick(void) const {
	return (nick.empty() ? "*": nick);
}

std::string	Client::getUser(void) const {
	return (user);
}

void	Client::setAuthenticated(void) { is_authenticated = true; }
bool	Client::isAuthenticated(void) const { return is_authenticated; }

void	Client::setRegistered(void){
	if (!is_registered) {
		is_registered = true;
		server.sendString(server.getFd(), getFd(), "001 " + getNick() + " :Registration complete!");
	}
}

bool	Client::isRegistered(void) const { return (is_registered && is_authenticated); }

std::string Client::validateUser( std::string user ) {
	if (!getUser().empty())
		throw std::invalid_argument("462 " + getNick() + " USER :You may not reregister!");
	if (user.empty())
		throw std::invalid_argument("461 " + getNick() + " USER :Can't set empty username!");
	if (user.size() > USER_MAX_LEN)
		user.erase(USER_MAX_LEN, std::string::npos);
	for (unsigned long i = 0; i < user.size(); i++) {
		if (!isalpha(user[i]) && !isdigit(user[i]) && (allowedchars.find(user[i]) == std::string::npos))
			throw std::invalid_argument("400 " + getNick()
				+ "USER :User name must not contain '" + std::string(1, user[i]) + "'");
	}
	if (caseInsCompare(user, "admin") || caseInsCompare(user, "root"))
		throw std::invalid_argument("400 " + getNick() + " USER :Inappropriate username");
	return (user);
}

std::string Client::validateNick( std::string nick ) {
	if (nick.empty())
		throw std::invalid_argument("433 " + getNick() + " NICK :Can't set an empty nick");
	if (server.getClient(nick))
		throw std::invalid_argument("433 " + getNick() + " NICK :Nick already taken on this server");
	if (!isalpha(nick[0]))
		throw std::invalid_argument("432 " + getNick() + " NICK :Nick must begin with a letter");
	if (nick.size() > NICK_MAX_LEN)
		nick.erase(NICK_MAX_LEN, std::string::npos);
	for (unsigned long i = 0; i < nick.size(); i++) {
		if (!isalpha(nick[i]) && !isdigit(nick[i]) && (allowedchars.find(nick[i]) == std::string::npos))
			throw std::invalid_argument("432 " + getNick()
					+ " NICK :Nick must not contain '" + std::string(1, nick[i]) + "'");
	}
	if (caseInsCompare(nick, "admin") || caseInsCompare(nick, "root")) // if same, returns true
		throw std::invalid_argument("432 " + getNick() + " NICK :Inappropriate nickname");
	return (nick);
}

void	Client::setNick(const std::string& in_nick) {
	nick = validateNick(in_nick);
	server.sendString(server.getFd(), getFd(), "Successfully set nickname to " + nick );
	if (!getUser().empty())
		setRegistered();
}

void	Client::setUser(const std::string& in_username) {
	user = validateUser(in_username);
	server.sendString(server.getFd(), getFd(), "Successfully set username to " + user );
	if (!getNick().empty() && getNick() != "*")
		setRegistered();
}

bool	Client::isInChannel( const Channel &channel ) const {
	return (channel.containsMember(*this));
}

bool	Client::isInChannel( const Channel *channel ) const {
	if (!channel)
		return (false);
	return (channel->containsMember(*this));
}
