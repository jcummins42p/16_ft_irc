/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/18 19:10:27 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <cstring>
#include <set>

class Client {
public:
	explicit Client(int fd);
	~Client( void );

	int			getFd(void) const;
	std::string	getNick(void) const;
	std::string	getUser(void) const;
	bool		getAutentificated(void) const;

	void		setAutentificated(void);
	void		setNick(const std::string& in_nick);
	void		setUser(const std::string& in_username);

	std::set<std::string>	channels;
private:
	const int				fd;
	std::string				nick;
	std::string				user;
	bool					is_autentificated;

};

#endif
