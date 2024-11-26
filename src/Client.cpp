/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/19 13:28:12 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


// Construct - destruct

Client::Client(int fd) : fd(fd), is_Authentificated(false) {}

Client::~Client( void ) {
	std::cout << "Reminder that you need to implement Client destructor" << std::endl;
}


// Getter - setters
const int&			Client::getFd(void) const { return (fd); }
const std::string&	Client::getNick(void) const { return nick; }
const std::string&	Client::getUser(void) const { return user; }
bool				Client::getAuthentificated(void) const { return is_Authentificated; }

void	Client::setAuthentificated(void) { is_Authentificated = true; }

void	Client::setNick(const std::string& in_nick) {
	if (in_nick.empty())
		std::cerr << "Can't set an empty nick!" << std::endl;
	else
		nick = in_nick;
}

void	Client::setUser(const std::string& in_username) {
	if (in_username.empty())
		std::cerr << "Can't set an empty username!" << std::endl;
	else
		user = in_username;
}

