/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/09 18:53:11 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


const static std::string allowedchars = "`|^_-{}[]\\";
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

static std::string validateUser( std::string name ) {
	if (name.empty())
		throw std::invalid_argument("Can't set an empty username!");
	if (name.size() > USER_MAX_LEN)
		name.erase(USER_MAX_LEN, std::string::npos);
	std::cout << "User: " << name << std::endl;
	for (unsigned long i = 0; i < name.size(); i++) {
		if (!isalpha(name[i]) && !isdigit(name[i]) && (allowedchars.find(name[i]) == std::string::npos))
			throw std::invalid_argument("User name must not contain '" + std::string(1, name[i]) + "'");
	}
	if (caseInsCompare(name, "admin") || caseInsCompare(name, "root"))
		throw std::invalid_argument("Inappropriate username");
	return (name);
}

static std::string validateNick( std::string nick ) {
	if (nick.empty())
		throw std::invalid_argument("Can't set an empty nick");
	if (!isalpha(nick[0]))
		throw std::invalid_argument("Nick must begin with a letter");
	if (nick.size() > NICK_MAX_LEN)
		nick.erase(NICK_MAX_LEN, std::string::npos);
	std::cout << "Nick: " << nick << std::endl;
	for (unsigned long i = 0; i < nick.size(); i++) {
		if (!isalpha(nick[i]) && !isdigit(nick[i]) && (allowedchars.find(nick[i]) == std::string::npos))
			throw std::invalid_argument("User name must not contain '" + std::string(1, nick[i]) + "'");
	}
	if (caseInsCompare(nick, "admin") || caseInsCompare(nick, "root")) // if same, returns true
		throw std::invalid_argument("Inappropriate nickname");
	return (nick);
}

void	Client::setNick(const std::string& in_nick) {
	nick = validateNick(in_nick);
	if (!getUser().empty())
		setRegistered();
}

void	Client::setUser(const std::string& in_username) {
	user = validateUser(in_username);
	if (!getNick().empty())
		setRegistered();
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
