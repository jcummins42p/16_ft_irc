/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/06 18:12:54 by jcummins         ###   ########.fr       */
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

class Client {
public:
	explicit Client(int fd, Server &server);
	~Client(void);

	const int&			getFd(void) const;
	const std::string&	getNick(void) const;
	const std::string&	getUser(void) const;
	bool				getAuthentificated(void) const;

	void				setAuthentificated(void);
	void				setNick(const std::string& in_nick);
	void				setUser(const std::string& in_username);
	const Server		&getServer( void );
	void				leaveChannel( const Channel &channel );

	bool	isInChannel( const Channel &channel ) const;
	bool	isInChannel( const Channel *channel ) const;
private:
	const int		fd;
	std::string		nick;
	std::string		user;
	bool			is_Authentificated;

	Server &server;
};

#endif
