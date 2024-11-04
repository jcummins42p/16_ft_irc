/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 14:58:10 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/04 16:56:29 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include "ircserv.hpp"

class Channel {
	private:
		std::string			name;
		std::string			key; // probably should be hashed, so we don't hold the actual key in memory
		std::string			topic;
		std::vector<Client>	clients;
		std::vector<Client>	operators;
		size_t				clnts_limit;
		bool				invite_only;
		bool				topic_operators_only;

		std::string	hashingFunc(const char* const str_to_hash) const;

	public:
		Channel(const std::string& in_name);
		~Channel();

		std::string	getName(void) const;
		std::string	getTopic(void) const;

		void		setTopic(const std::string& in_topic);
		void		setKey(const std::string& in_key);

		bool		addClient(const Client* const in_client); // bool here and in other functions to check if an action was successfull, could be changed to exceptions
		bool		addOperator(const Client* const in_operator);

		bool		kickClient(const Client* const in_client);
		bool		kickOperator(const Client* const in_operator);

		bool		changeMode(const char* const in_mode, Client* in_client); // should check if the client is in operators list
};

#endif
