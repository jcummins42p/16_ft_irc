/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 14:58:10 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/05 15:26:42 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#define CHANNEL_ERROR -1

#include <iostream>
#include "ircserv.hpp"

class Channel {
	private:
		std::string					name;
		std::string					key; // probably should be hashed, so we don't hold the actual key in memory
		std::string					topic;
		std::vector<const Client*>	clients;
		std::vector<const Client*>	admins;
		size_t						clnts_limit;
		bool						invite_only;
		bool						topic_admins_only;

		std::string	hashingFunc(const std::string& str_to_hash) const;
		int			posInList(const std::vector<const Client*>& list, const Client& in_client) const;
		bool		notAnAdmin(void) const;

	public:
		Channel(const std::string& in_name, const Client& creator);
		~Channel();

		std::string	getName(void) const;
		std::string	getTopic(void) const;


		// bool here and in other functions to check if an action was successfull, could be changed to exceptions
		bool		setTopic(const std::string& in_topic, const Client& admin);
		bool		setKey(const std::string& in_key, const Client& admin);

		bool		addClient(const Client& in_client, const Client& admin);
		bool		addAdmin(const Client& in_client, const Client& admin);

		bool		kickClient(const Client& in_client, const Client& admin);
		bool		kickAdmin(const Client& in_client, const Client& admin);

		bool		joinChannel(const Client& in_client);
		bool		leaveChannel(const Client& in_client);

		bool		changeMode(const char* const in_mode, const Client& in_client); // should check if the client is in admins list
};

#endif
