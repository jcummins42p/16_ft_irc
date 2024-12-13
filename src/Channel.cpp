/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:00:44 by mmakagon          #+#    #+#             */
/*   Updated: 2024/12/13 14:02:29 by jcummins         ###   ########.fr       */
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
	if (name.empty())
		throw std::invalid_argument("Channel name cannot be empty");
	if (name[0] != '#' && name[0] != '&')
		throw std::invalid_argument("Static Channel name must begin with '#' or '&'.");
	if (name.size() == 1)
		throw std::invalid_argument("Channel name cannot be empty");
	for (unsigned long i = 0; i < name.size(); i++) {
		if (!isalnum(name[i]) && (allowedchars.find(name[i]) == std::string::npos))
			throw std::invalid_argument("User name must not contain '" + std::string(1, name[i]) + "'");
	}
}

Channel::Channel( Server &server, std::string in_name, const Client& creator, const std::string& password) :
	server(server),
	clnts_limit(MAX_CLIENTS),
	invite_only(false),
	topic_admins_only(false),
	topic_set(false),
	locked(true),
	secret(false)
{
	if (in_name.empty())
		in_name = "#Default";
	validateName(in_name);
	name = in_name;
	if (server.getChannel(in_name))
		throw std::invalid_argument(name + " already in use ");

	if (password.empty())
		locked = false;
	else
		hashed_pass = hashSimple(password);  // Set the hashed password for the channel.
	clients.insert(&creator);		   // Add the creator as the first client.
	admins.insert(&creator);			// Set the creator as the initial admin.
}

Channel::~Channel(void) {}

/* PRIVATE METHODS */

void Channel::internalMessage(const Client &client, const std::string &message) const {
	server.sendString(client.getFd(), message.c_str());
}

/* GETTERS */

const std::string&	Channel::getName(void) const { return (name); }
const std::string&	Channel::getTopic(void) const { return (topic); }
bool Channel::hasTopic(void) const { return (topic_set); }
bool Channel::isSecret(void) const { return (secret); }

/* SETTERS */

void	Channel::setTopic(const std::string &in_topic, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (topic_admins_only && admins.find(&admin) == admins.end())
		throw std::runtime_error ("Admin rights required");
	else if (in_topic.empty()) {
		throw std::runtime_error ("Can't set an empty topic");
	}
	topic = in_topic;
	topic_set = true;
	channelMessage(getName() + " topic changed by "
			+ admin.getNick() + " to '"
			+ in_topic + "'", admin);
}

void	Channel::setPass(const std::string &in_pass, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error ("Admin rights required");
	else if (in_pass.empty())
		throw std::runtime_error ("Can't set an empty password!");
	else if (in_pass.size() > MAX_PASS_LEN)
		throw std::runtime_error ("Password length cannot exceed " + intToString(MAX_PASS_LEN));
	hashed_pass = hashSimple(in_pass);
}

void Channel::setUserLimit(const long &newlimit, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error ("Admin rights required");
	if (newlimit < 1)
		throw std::runtime_error ("Minimum user limit is 1");
	if (newlimit > static_cast<long>(SIZE_MAX))
		throw std::runtime_error ("Maximum user limit exceeded");
	clnts_limit = static_cast<size_t>(newlimit);
}

/* ADMIN FUNCTIONS */

void	Channel::addClient(const Client &target, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error ("Admin rights required");
	if (clients.size() >= clnts_limit)
		throw std::runtime_error ("The channel is full!");
	if (clients.find(&target) != clients.end())
		throw std::runtime_error("Already in the channel!");
	if (banned_clients.find(&target) != banned_clients.end())
		throw std::runtime_error("Banned from the channel!");
	clients.insert(&target);
}

void	Channel::addAdmin(const Client &target, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error("Admin rights required");
	if (admins.find(&target) != admins.end())
		throw std::runtime_error(target.getNick() + " is already admin!");
	if (clients.find(&target) == clients.end())
		throw std::runtime_error("Not in the channel!");
	admins.insert(&target);
}

void	Channel::revokeAdmin(const Client &target, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error("Admin rights required");
	if (clients.find(&target) == clients.end())
		throw std::runtime_error("Not in the channel!");
	if (admins.find(&target) == admins.end())
		throw (std::runtime_error("Target is not admin!"));
	admins.erase(&target);
}

void Channel::kickClient(const Client &target, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw (std::runtime_error("Admin rights required"));
	if (clients.find(&target) == clients.end())
		throw (std::runtime_error("Target is not in channel"));
	if (admins.find(&target) != admins.end())
		throw (std::runtime_error("Cannot kick admin"));
	internalMessage(target, getName() + ": You have been kicked by "
							+ admin.getNick() + ", bye.");
	removeClient( target );
}

void Channel::banClient(const Client &toban, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (clients.find(&toban) != clients.end())
		kickClient(toban, admin);
	if (banned_clients.find(&toban) != banned_clients.end())
		revokeInvite(toban, admin);
	banned_clients.insert(&toban);  // Add the client to the invited list.
	internalMessage(toban, "You have been banned from " + name);
}

void	Channel::revokeBan(const Client &unban, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error("Admin rights required");
	if (banned_clients.find(&unban) == banned_clients.end())
		throw (std::runtime_error("User is no longer banned"));
	banned_clients.erase(&unban);
}

/* JOIN - LEAVE */

void Channel::joinChannel(const Client &target, const std::string &password) {
	// Check if the channel is invite-only and if the client is invited.
	if (banned_clients.find(&target) != banned_clients.end())
		throw std::runtime_error(getName() + ": " + target.getNick() + " is banned!");
	if (clients.find(&target) != clients.end())
		throw std::runtime_error(getName() + ": Already a member!");
	if (invite_only && invited_clients.find(&target) == invited_clients.end())
		throw std::runtime_error(getName() + " is invite-only!");
	if (clients.size() >= clnts_limit) // If the channel is full, deny the client.
		throw std::runtime_error(getName() + " is already full!");
	if (locked) { // If the channel isn't invite-only, check the password.
		if (hashed_pass != hashSimple(password)) {
			throw std::runtime_error(getName() + ": Incorrect password");
		}
	}
	// Add the client to the channel.
	clients.insert(&target);
	channelMessage(getName() + ": " + target.getNick() + " joined the channel!", target);
}

void Channel::removeClient(const Client &target) {
	admins.erase(&target);  // Remove admin rights if applicable.
	clients.erase(&target); // Remove the client from the channel.

	internalMessage(target, getName() + ": You left the channel");
	// Reassign admin rights if the last admin leaves:
	if (admins.empty() && !clients.empty()) {
		admins.insert(*clients.begin());  // Assign the first client as the new admin
		internalMessage(**clients.begin(), getName() + ": You are now channel admin");
	}
	// If there are no clients left, you can clean up or close the channel.
	if (clients.empty()) {
		throw std::exception();
	}
}

/* GROUP MESSAGES */

void Channel::channelMessage(const std::string &message, const Client &sender) {
	if (clients.find(&sender) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	for (std::set<const Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		// Don't send the message back to the sender
		if (*it != &sender)
			server.sendString((*it)->getFd(), message);
	}
}

/* INVITE MANAGEMENT */

void Channel::inviteClient(const Client &invited, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw (std::runtime_error("Admin rights required"));
	if (clients.find(&invited) != clients.end())
		throw (std::runtime_error("User is already in channel"));
	if (invited_clients.find(&invited) != invited_clients.end())
		throw (std::runtime_error("User is already invited to channel"));
	if (banned_clients.find(&invited) != banned_clients.end())
		revokeBan(invited, admin);
	invited_clients.insert(&invited);  // Add the client to the invited list.
	internalMessage(invited, "You have been invited to join " + name);
}

void	Channel::revokeInvite(const Client &target, const Client &admin) {
	if (clients.find(&admin) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	if (admins.find(&admin) == admins.end())
		throw std::runtime_error("Admin rights required");
	if (invited_clients.find(&target) == invited_clients.end())
		throw (std::runtime_error("User is not invited!"));
	invited_clients.erase(&target);
}

/* INFO */

bool Channel::containsMember(const Client &client ) const {
	if (clients.find(&client) != clients.end())
		return (true);
	return (false);
}
