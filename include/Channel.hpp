/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 14:58:10 by mmakagon          #+#    #+#             */
/*   Updated: 2024/12/13 17:09:49 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>
#include "Client.hpp"
#include <sys/types.h>
#include "ft_irc.hpp"

//#define MAX_CLIENTS 10
#define MAX_PASS_LEN 16
#define SIZE_MAX 512

class Channel {
private:
	Server &server;

    std::string		name;
    std::string		topic;
    unsigned int	hashed_pass;
	//	MODE SWITCHES
    size_t	clnts_limit;
    bool 	invite_only;
    bool	topic_admins_only;
	bool	topic_set;
	bool	locked;
	bool	secret;
	//	CLIENT LISTS
    std::set<const Client*> clients;
    std::set<const Client*> admins;
    std::set<const Client*> invited_clients; // Declare the invited clients.
    std::set<const Client*> banned_clients;

	void internalMessage(const Client &client, const std::string &message) const;

	// Mode function map
	typedef std::string (Channel::*ChanModeHandler)(int, const std::string &, bool);
	std::map<char, ChanModeHandler> modeHandlers;
	// Mode controls
	std::string handleModeInvite(int client_fd, const std::string &input, bool toggle);
	std::string handleModeTopic(int client_fd, const std::string &input, bool toggle);
	std::string handleModeKey(int client_fd, const std::string &input, bool toggle);
	std::string	handleModeOperator(int client_fd, const std::string &input, bool toggle);
	std::string handleModeUserLimit(int client_fd, const std::string &input, bool toggle);
	std::string handleModeSecret(int client_fd, const std::string &input, bool toggle);

public:
    // Constructor / Destructor
    Channel(Server &server, std::string in_name, const Client &creator, const std::string &password);
    ~Channel();

	static void validateName( const std::string &name );

    // Topic management
    const std::string &getName(void) const;
    const std::string &getTopic(void) const;
    void setTopic(const std::string& in_topic, const Client& admin);
	bool hasTopic(void) const;
	bool isSecret(void) const;

    // Password management
    void setPass(const std::string &in_pass, const Client& admin);

    // Channel management
    void addClient(const Client &target, const Client &admin);
    void removeClient(const Client &target);
	// Admin rights
    void addAdmin(const Client &target, const Client &admin);
    void revokeAdmin(const Client &target, const Client &admin);
    void kickClient(const Client &target, const Client &admin);
	void setUserLimit(const long &newlimit, const Client &admin);
	// Ban management
	void banClient(const Client &toban, const Client &admin);
	void revokeBan(const Client &unban, const Client &admin);

    // Join/Leave
    void joinChannel(const Client &target, const std::string &password); // Updated declaration.
	void create(const Client &creator, const std::string &password); //

    // Invite management
    void inviteClient(const Client &target, const Client &admin); // Declare the inviteClient method.
    void revokeInvite(const Client &target, const Client &admin); // Declare the inviteClient method.

    // Group messaging
    void channelMessage( const std::string &message, const Client &sender) ;

	// Info
	bool containsMember(const Client &client ) const ;

	//	Mode command selector
	std::string modeHandler(int client_fd, std::istringstream &iss);
};

#endif
