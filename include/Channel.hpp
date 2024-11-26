/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 14:58:10 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/19 11:52:11 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <set>
#include "Client.hpp"
#include "ft_irc.hpp"

class Channel {
	private:
		std::string					name;
		std::string					topic;
		unsigned int				hashed_pass;
		std::set<const Client*>		clients;
		std::set<const Client*>		admins;
		size_t						clnts_limit;
		bool						invite_only;
		bool						topic_admins_only;

		ssize_t		internalMessage(const Client& client, const std::string message) const;

	public:
		Channel(const std::string& in_name, const Client& creator);
		~Channel();

		const std::string&	getName(void) const;
		const std::string&	getTopic(void) const;


		// bool here and in other functions to check if an action was successfull, could be changed to exceptions
		bool		setTopic(const std::string& in_topic, const Client& admin);
		bool		setPass(std::string& in_pass, const Client& admin);

		bool		addClient(const Client& in_client, const Client& admin);
		bool		addAdmin(const Client& in_client, const Client& admin);

		bool		kickClient(const Client& in_client, const Client& admin);
		bool		kickAdmin(const Client& in_client, const Client& admin);

		bool		joinChannel(const Client& in_client);
		bool		leaveChannel(const Client& in_client);

		bool		changeMode(const char* const in_mode, const Client& admin);
		void		channelMessage(const std::string message, const Client& sender) const;
};

#endif
