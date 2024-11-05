/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/05 18:31:04 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

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

class Channel {
	public:
		Channel(const std::string& name);
		//Channel ( const Channel &other );
		~Channel( void );

		std::string name;
		std::set<int> clients; //client file descriptors
		std::string topic;
		std::string password;
		bool invite_only;
		bool topic_restricted;
		int user_limit;

		void join(int client_fd);
		void part(int client_fd);
		void sendMessage(const std::string& message, int sender_fd);
		void setTopic(const std::string& new_topic);
		std::string getTopic() const;
		void setPassword(const std::string& pass);
		bool isOperator(int client_fd);
};

#endif
