/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel_old.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/19 11:44:57 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <set>
#include <arpa/inet.h>

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
		void channelMessage(const std::string& message, int sender_fd);
		void setTopic(const std::string& new_topic);
		std::string getTopic() const;
		void setPassword(const std::string& pass);
		bool isOperator(int client_fd);
};

#endif
