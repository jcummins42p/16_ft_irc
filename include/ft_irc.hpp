/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 13:00:32 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/18 13:16:27 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCSERV_HPP
# define IRCSERV_HPP

class Channel;
class IRCServer;
class Client;

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <poll.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <fcntl.h>

# define MAX_CLIENTS 100
# define BUFFER_SIZE 512

# include "Channel.hpp"
# include "Client.hpp"
# include "IRCServer.hpp"

unsigned int	hashSimple(std::string& in_str);

#endif
