/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:00:44 by mmakagon          #+#    #+#             */
/*   Updated: 2024/12/11 19:32:28 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <sys/types.h>
#include "ft_irc.hpp"
#include <sys/socket.h>


const static std::string allowedchars = "`|^_-{}[]\\!@#$%&*()+=.";

/* CONSTRUCT - DESTRUCT */

//	This is a static class function to check if a server name is valid to search for
void Channel::validateName(const std::string &name ) {
	if (name[0] != '#' && name[0] != '&')
		throw std::invalid_argument("Static Channel name must begin with '#' or '&'.");
	for (unsigned long i = 0; i < name.size(); i++) {
		if (!isalnum(name[i]) && (allowedchars.find(name[i]) == std::string::npos))
			throw std::invalid_argument("User name must not contain '" + std::string(1, name[i]) + "'");
	}
}

//	This is used during construction to check that the name is legal and available
static void validateNameHelper(const Server &server, const std::string &name ) {
	if (name[0] != '#' && name[0] != '&')
		throw std::invalid_argument("Helper Channel name must begin with '#' or '&'.");
	for (unsigned long i = 0; i < name.size(); i++) {
		if (!isalnum(name[i]) && (allowedchars.find(name[i]) == std::string::npos))
			throw std::invalid_argument("User name must not contain '" + std::string(1, name[i]) + "'");
	}
	if (server.getChannel(name))
		throw std::invalid_argument(name + " already in use ");
}

Channel::Channel( Server &server, const std::string& in_name, const Client& creator, const std::string& password) :
	server(server),
	clnts_limit(MAX_CLIENTS),
	invite_only(false),
	topic_admins_only(true),
	topic_set(false),
	pass_required(true)
{
	if (in_name.empty())
		name = "#Default";
	else
		name = in_name;
	validateNameHelper(server, name);

	if (password.empty())
		pass_required = false;
	else
		hashed_pass = hashSimple(password);  // Set the hashed password for the channel.
	clients.insert(&creator);		   // Add the creator as the first client.
	admins.insert(&creator);			// Set the creator as the initial admin.
}

Channel::~Channel(void) {}

/* PRIVATE METHODS */

void Channel::internalMessage(const Client &client, const std::string &message) const {
	return server.sendString(client.getFd(), message.c_str());
}

/* GETTERS */

const std::string&	Channel::getName(void) const { return (name); }
const std::string&	Channel::getTopic(void) const { return (topic); }
bool Channel::hasTopic(void) const { return (topic_set); }

/* SETTERS */

void	Channel::setTopic(const std::string &in_topic, const Client &admin) {
	if (topic_admins_only && admins.find(&admin) == admins.end())
		throw std::runtime_error ("Admin rights required");
	else if (in_topic.empty()) {
		throw std::runtime_error ("Can't set an empty topic");
	}
	topic = in_topic;
	topic_set = true;
}

void	Channel::setPass(const std::string &in_pass, const Client &admin) {
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error ("Admin rights required");
	else if (in_pass.empty())
		throw std::runtime_error ("Can't set an empty password!");
	else if (in_pass.size() > MAX_PASS_LEN)
		throw std::runtime_error ("Password length cannot exceed " + intToString(MAX_PASS_LEN));
	hashed_pass = hashSimple(in_pass);
}

/* ADMIN FUNCTIONS */

void	Channel::addClient(const Client &in_client, const Client &admin) {
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error ("Admin rights required");
	else if (clients.size() >= clnts_limit)
		throw std::runtime_error ("The channel is full!");
	clients.insert(&in_client);
}

void	Channel::addAdmin(const Client &in_client, const Client &admin) {
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error("Admin rights required");
	else if (clients.find(&in_client) == clients.end())
		throw std::runtime_error("User is not in the channel!");
	admins.insert(&in_client);
}

void Channel::kickClient(const Client &target, const Client &admin) {
	if (admins.find(&admin) == admins.end())
		throw (std::runtime_error("Admin rights required"));
	if (clients.find(&target) == clients.end())
		throw (std::runtime_error("User is not in channel"));
	if (admins.find(&target) != admins.end())
		throw (std::runtime_error("Cannot kick admin"));
	internalMessage(target, "You have been kicked from " + getName()
							+ " by " + admin.getNick() + ", bye.");
	removeClient( target );
}

void Channel::kickAdmin(const Client &target, const Client &admin) {
	if (admins.find(&admin) == admins.end())
		throw (std::runtime_error("Admin rights required"));
	else if (clients.find(&target) == clients.end())
		throw (std::runtime_error("User not in channel"));
	else if (admins.find(&target) == admins.end())
		throw (std::runtime_error("Target is not admin"));
	admins.erase(&target);  // Remove the user from the admin list

	// If all admins are removed, assign a new admin if there are clients left.
	if (admins.empty() && !clients.empty()) {
		// Assign the first client as the new admin
		admins.insert(*clients.begin());
		internalMessage(**clients.begin(), "You are now the channel admin.");
	}
}

/* JOIN - LEAVE */

void Channel::joinChannel(const Client &in_client, const std::string &password) {
	// Check if the channel is invite-only and if the client is invited.
	if (clients.find(&in_client) != clients.end())
		throw std::runtime_error("Already a member of " + getName());
	if (invite_only && invited_clients.find(&in_client) == invited_clients.end())
		throw std::runtime_error(getName() + " is invite-only!");
	if (clients.size() >= clnts_limit) // If the channel is full, deny the client.
		throw std::runtime_error(getName() + " is already full!");
	if (!invite_only && pass_required) { // If the channel isn't invite-only, check the password.
		if (hashed_pass != hashSimple(password)) {
			throw std::runtime_error("Incorrect password for " + getName());
		}
	}
	// Add the client to the channel.
	clients.insert(&in_client);
}

void Channel::removeClient(const Client &in_client) {
	admins.erase(&in_client);  // Remove admin rights if applicable.
	clients.erase(&in_client); // Remove the client from the channel.

	// Reassign admin rights if the last admin leaves:
	if (admins.empty() && !clients.empty()) {
		admins.insert(*clients.begin());  // Assign the first client as the new admin
		internalMessage(**clients.begin(), "You are now channel admin for " + getName());
	}
	// If there are no clients left, you can clean up or close the channel.
	if (clients.empty()) {
		throw std::exception();
	}
}

/* GROUP MESSAGES */

void Channel::channelMessage(const std::string &message, const Client &sender) {
	for (std::set<const Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		// Don't send the message back to the sender
		if (*it != &sender)
			server.sendString((*it)->getFd(), message);
	}
}

/* INVITE MANAGEMENT */

void Channel::inviteClient(const Client &in_client, const Client &admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!");
		return;
	}
	invited_clients.insert(&in_client);  // Add the client to the invited list.
	internalMessage(in_client, "You have been invited to join the channel: " + name);
}

/* INFO */

bool Channel::containsMember(const Client &client ) const {
	if (clients.find(&client) != clients.end())
		return (true);
	return (false);
}
