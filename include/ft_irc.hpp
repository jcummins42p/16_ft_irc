/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 13:00:32 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/27 14:56:54 by pyerima          ###   ########.fr       */
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

#endif
