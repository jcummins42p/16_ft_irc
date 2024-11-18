/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 13:00:32 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/18 18:44:47 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IRCSERV_HPP
# define IRCSERV_HPP

class Channel;
class IRCServer;
class Client;

# include <poll.h>
# include <unistd.h>

# define MAX_CLIENTS 100
# define BUFFER_SIZE 512

# include "Channel.hpp"
# include "Client.hpp"
# include "IRCServer.hpp"

unsigned int	hashSimple(std::string& in_str);

#endif
