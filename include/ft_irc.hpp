/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 13:00:32 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/04 17:28:08 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCSERV_HPP
# define IRCSERV_HPP

class Channel;
class Server;
class Client;

# include <poll.h>
# include <cstdlib>
# include <unistd.h>

# define MAX_CLIENTS 100
# define BUFFER_SIZE 512

# include "Channel.hpp"
# include "Client.hpp"
# include "Server.hpp"

unsigned int	hashSimple(const std::string& in_str);

//std::string intToString(int value) {
	//char buffer[20];
	//std::sprintf(buffer, "%d", value);
	//return std::string(buffer);
//}

#endif
