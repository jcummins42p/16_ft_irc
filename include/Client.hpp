/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/04 15:26:41 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <cstring>
#include <set>

class Server;

class Client {
public:
	explicit Client(int fd, const Server &server);
	~Client(void);

	const int&			getFd(void) const;
	const std::string&	getNick(void) const;
	const std::string&	getUser(void) const;
	bool				getAuthentificated(void) const;

	void				setAuthentificated(void);
	void				setNick(const std::string& in_nick);
	void				setUser(const std::string& in_username);
	const Server		&getServer( void );

	std::set<std::string>	channels;
private:
	const int		fd;
	std::string		nick;
	std::string		user;
	bool			is_Authentificated;

	const Server &server;
};

#endif
