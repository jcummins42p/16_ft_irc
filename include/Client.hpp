/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/18 17:31:10 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <cstring>
#include <set>
#include "Channel.hpp"

class Server;
class Channel;

#define NICK_MAX_LEN 12
#define USER_MAX_LEN 12

class Client {
public:
	explicit Client(int fd, Server &server);
	~Client(void);

	const int&	getFd(void) const;
	std::string	getNick(void) const;
	std::string	getUser(void) const;
	std::string validateUser( std::string user );
	std::string validateNick( std::string nick );

	void	setAuthenticated(void);
	bool	isAuthenticated(void) const;

	void	setRegistered(void);
	bool	isRegistered(void) const;

	void	setNick(const std::string& in_nick);
	void	setUser(const std::string& in_username);
	void	leaveChannel( const Channel &channel );

	bool	isInChannel( const Channel &channel ) const;
	bool	isInChannel( const Channel *channel ) const;
private:
	const int		fd;
	std::string		nick;
	std::string		user;
	bool			is_authenticated;
	bool			is_registered;

	const static std::string namechars;

	Server &server;
};

#endif
