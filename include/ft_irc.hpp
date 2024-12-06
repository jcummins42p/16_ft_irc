/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_irc.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 13:00:32 by pyerima           #+#    #+#             */
/*   Updated: 2024/12/06 10:08:49 by jcummins         ###   ########.fr       */
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

# include "utils.hpp"
# include "Channel.hpp"
# include "Client.hpp"
# include "Server.hpp"
# include "Logger.hpp"

#endif
