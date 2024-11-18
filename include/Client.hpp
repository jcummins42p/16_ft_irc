/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/18 14:46:12 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdlib.h>

class Client {
public:
	Client(int fd);
	~Client( void );


	int						fd;
	std::string				nick;
	std::string				user;
	std::set<std::string>	channels;

	bool					is_autentificated;
};

#endif
