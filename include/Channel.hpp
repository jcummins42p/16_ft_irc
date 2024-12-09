/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 14:58:10 by mmakagon          #+#    #+#             */
/*   Updated: 2024/12/09 17:10:54 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include "Client.hpp"
#include <sys/types.h>
#include "ft_irc.hpp"

//#define MAX_CLIENTS 10

class Channel {
private:
	Server &server;

    std::string		name;
    std::string		topic;
    unsigned int	hashed_pass;
    size_t			clnts_limit;
    bool 			invite_only;
    bool			topic_admins_only;
    std::set<const Client*> clients;
    std::set<const Client*> admins;
    std::set<const Client*> invited_clients; // Declare the invited clients.
											 //
    void internalMessage(const Client& client, const std::string message) const;

public:
    // Constructor
    Channel(Server &server, const std::string& in_name, const Client& creator, const std::string& password);

    // Destructor
    ~Channel();

    // Topic management
    const std::string& getName(void) const;
    const std::string& getTopic(void) const;
    bool setTopic(const std::string& in_topic, const Client& admin);

    // Password management
    bool setPass(std::string& in_pass, const Client& admin);

    // Channel management
    bool addClient(const Client& in_client, const Client& admin);
    bool addAdmin(const Client& in_client, const Client& admin);
    bool kickClient(const Client& target, const Client& admin);
    bool kickAdmin(const Client& target, const Client& admin);

    // Join/Leave
    bool joinChannel(const Client& in_client, const std::string& password); // Updated declaration.
    bool leaveChannel(const Client& in_client);
	void create(const Client& creator, const std::string& password); //

    // Invite management
    void inviteClient(const Client& in_client, const Client& admin); // Declare the inviteClient method.

    // Group messaging
    void channelMessage( const std::string &message, const Client &sender) ;

	// Info
	bool containsMember(const Client &client ) const ;
};

#endif
