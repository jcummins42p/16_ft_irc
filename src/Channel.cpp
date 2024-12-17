/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:00:44 by mmakagon          #+#    #+#             */
/*   Updated: 2024/12/17 21:07:01 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <sys/types.h>
#include "ft_irc.hpp"
#include <sys/socket.h>

const static std::string allowedchars = "`|^_-{}[]\\!@#$%&*()+=.";
const static std::string allowedprefix = "#&!+";

/* CONSTRUCT - DESTRUCT */

//	This is a static class function to check if a server name is valid to search for
void Channel::validateName(const std::string &name ) {
	if (name.empty())
		throw std::invalid_argument("Channel name cannot be empty");
	if (allowedprefix.find(name[0]) == std::string::npos)
		throw std::invalid_argument("Channel name must begin with: # & ! +");
	if (name.size() == 1)
		throw std::invalid_argument("Channel name cannot be empty");
	for (unsigned long i = 1; i < name.size(); i++) {
		if (!isalnum(name[i]) && (allowedchars.find(name[i]) == std::string::npos))
			throw std::invalid_argument("User name must not contain '" + std::string(1, name[i]) + "'");
	}
}

Channel::Channel(Server &server, std::string in_name, const Client& creator, const std::string& password) :
	server(server),
	name(in_name),
	clnts_limit(MAX_CLIENTS),
	invite_only(false),
	topic_admins_only(false),
	topic_set(false),
	locked(true),
	secret(false)
{
	if (password.empty())
		locked = false;
	else
		hashed_pass = hashSimple(password);  // Set the hashed password for the channel.
	owner = &creator;			//	Add creator as channel owner.
	clients.insert(&creator);		   // Add the creator as the first client.
	admins.insert(&creator);			// Set the creator as the initial admin.
	//	Set mode handler function map
	modeHandlers['i'] = &Channel::handleModeInvite;
	modeHandlers['t'] = &Channel::handleModeTopic;
	modeHandlers['k'] = &Channel::handleModeKey;
	modeHandlers['o'] = &Channel::handleModeOperator;
	modeHandlers['l'] = &Channel::handleModeUserLimit;
	modeHandlers['s'] = &Channel::handleModeSecret;
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

void	Channel::checkRights(const Client &executor, e_privlevel level) {
	if (level >= MEMBER && clients.find(&executor) == clients.end())
		throw (std::runtime_error("Not in the channel"));
	else if (level >= ADMIN && admins.find(&executor) == admins.end())
		throw std::runtime_error ("Admin rights required");
	else if (level == OWNER && &executor != owner)
		throw std::runtime_error ("Channel owner rights required");
}

void	Channel::setTopic(const std::string &in_topic, const Client &admin) {
	if (topic_admins_only)
		checkRights(admin, ADMIN);
	else
		checkRights(admin, MEMBER);
	if (in_topic.empty())
		throw std::runtime_error ("Can't set an empty topic");
	topic = in_topic;
	topic_set = true;
	channelMessage(getName() + " topic changed by "
			+ admin.getNick() + " to '"
			+ in_topic + "'", admin);
}

void	Channel::setPass(const std::string &in_pass, const Client &admin) {
	checkRights(admin, ADMIN);
	if (in_pass.empty())
		throw std::runtime_error ("Can't set an empty password!");
	else if (in_pass.size() > MAX_PASS_LEN)
		throw std::runtime_error ("Password length cannot exceed " + intToString(MAX_PASS_LEN));
	hashed_pass = hashSimple(in_pass);
}

void Channel::setUserLimit(const long &newlimit, const Client &admin) {
	checkRights(admin, ADMIN);
	if (newlimit < 1)
		throw std::runtime_error ("Minimum user limit is 1");
	if (newlimit > static_cast<long>(SIZE_MAX))
		throw std::runtime_error ("User limit cannot exceed " + intToString(SIZE_MAX));
	clnts_limit = static_cast<size_t>(newlimit);
}

/* ADMIN FUNCTIONS */

void	Channel::addAdmin(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (admins.find(&target) != admins.end())
		throw std::runtime_error(target.getNick() + " is already admin!");
	if (clients.find(&target) == clients.end())
		throw std::runtime_error("Not in the channel!");
	admins.insert(&target);
	internalMessage(target, name + ": You have been granted admin by " + admin.getNick());
}

void	Channel::revokeAdmin(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (clients.find(&target) == clients.end())
		throw std::runtime_error(target.getNick() + " not in the channel!");
	if (admins.find(&target) == admins.end())
		throw (std::runtime_error("Target is not admin!"));
	admins.erase(&target);
	internalMessage(target, name + ": Your admin rights were revoked by " + admin.getNick());
}

void Channel::kickClient(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (clients.find(&target) == clients.end())
		throw (std::runtime_error(target.getNick() + " is not in channel"));
	if (owner != &admin && (admins.find(&target) != admins.end()))
		throw (std::runtime_error("Cannot kick admin"));
	internalMessage(target, getName() + ": You have been kicked by "
							+ admin.getNick() + ", bye.");
	removeClient( target );
}

void Channel::banClient(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (clients.find(&target) != clients.end())
		kickClient(target, admin);
	if (invited_clients.find(&target) != invited_clients.end())
		revokeInvite(target, admin);
	banned_clients.insert(&target);  // Add the client to the invited list.
	internalMessage(target, name + ": You have been banned by " + admin.getNick());
	internalMessage(admin, name + ": You banned " + target.getNick());
}

void	Channel::revokeBan(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (banned_clients.find(&target) == banned_clients.end())
		throw (std::runtime_error(target.getNick() + " is not on ban list"));
	internalMessage(target, name + ": Your ban has been revoked by " + admin.getNick());
	internalMessage(admin, name + ": You unbanned " + target.getNick());
	banned_clients.erase(&target);
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
	internalMessage(target, name + ": You have joined the channel");
	if (hasTopic())
		internalMessage(target, "Topic: " + getTopic());
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
	if (owner == &target && !admins.empty()) { // if owner leaves, appoint another admin
		owner = *admins.begin();
		internalMessage(*owner, getName() + ": You are now channel owner");
	}
	// If there are no clients left, you can clean up or close the channel.
	if (clients.empty()) {
		throw std::exception();
	}
}

/* GROUP MESSAGES */

void Channel::channelMessage(const std::string &message, const Client &sender) {
	checkRights(sender, MEMBER);
	for (std::set<const Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		// Don't send the message back to the sender
		if (*it != &sender)
			server.sendString((*it)->getFd(), message);
	}
}

/* INVITE MANAGEMENT */

void Channel::inviteClient(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (clients.find(&target) != clients.end())
		throw (std::runtime_error(target.getNick() + " is already in channel"));
	if (invited_clients.find(&target) != invited_clients.end())
		throw (std::runtime_error(target.getNick() + " is already invited to channel"));
	if (banned_clients.find(&target) != banned_clients.end())
		revokeBan(target, admin);
	invited_clients.insert(&target);  // Add the client to the invited list.
	internalMessage(target, name + ": You have been invited to join by " + admin.getNick());
	internalMessage(admin, name + ": You invited " + target.getNick());
}

void	Channel::revokeInvite(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (invited_clients.find(&target) == invited_clients.end())
		throw (std::runtime_error(target.getNick() + " is not on invite list!"));
	invited_clients.erase(&target);
	internalMessage(target, name + ": Your invite has been revoked by " + admin.getNick());
}

/* INFO */

bool Channel::containsMember(const Client &client ) const {
	if (clients.find(&client) != clients.end())
		return (true);
	return (false);
}
