/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 14:58:10 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/04 15:30:05 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include "ircserv.hpp"

class Channel {
	private:
		std::string			name;
		std::string			key;
		std::string			topic;
		std::vector<Client>	clients;
		std::vector<Client>	operators;
		size_t				clnts_limit;
		bool				invite_only;
		bool				topic_operators_only;

	public:
		Channel(const std::string& in_name);
		~Channel();

		std::string	getName(void) const;
		std::string	getTopic(void) const;

		void		setTopic(const std::string& in_topic);
		void		setKey(const std::string& in_key);

		bool		addClient(const Client* const in_client);
		bool		addOperator(const Client* const in_operator);

		bool		kickClient(const Client* const in_client);
		bool		kickOperator(const Client* const in_operator);

		bool		changeMode(char* in_mode);
};

#endif
