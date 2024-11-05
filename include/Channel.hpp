/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.com>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 14:58:10 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/04 18:59:51 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include "ircserv.hpp"

class Channel {
	private:
		std::string				name;
		std::string				key; // probably should be hashed, so we don't hold the actual key in memory
		std::string				topic;
		std::vector<Client*>	clients;
		std::vector<Client*>	admins;
		size_t					clnts_limit;
		bool					invite_only;
		bool					topic_admins_only;

		std::string	hashingFunc(const std::string& str_to_hash) const;
		bool		isAdmin(const Client& in_client);

	public:
		Channel(const std::string& in_name, Client& creator);
		~Channel();

		std::string	getName(void) const;
		std::string	getTopic(void) const;

		bool		setTopic(const std::string& in_topic, const Client& modifier);
		bool		setKey(const std::string& in_key, const Client& modifier);

		bool		addClient(const Client& in_client, const Client& modifier); // bool here and in other functions to check if an action was successfull, could be changed to exceptions
		bool		addAdmin(const Client& in_admin, const Client& modifier);

		bool		kickClient(const Client& in_client, const Client& modifier);
		bool		kickAdmin(const Client& in_admin, const Client& modifier);

		bool		changeMode(const char* const in_mode, const Client& in_client); // should check if the client is in admins list
};

#endif
