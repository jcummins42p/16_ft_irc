/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:00:44 by mmakagon          #+#    #+#             */
/*   Updated: 2024/12/19 17:20:14 by jcummins         ###   ########.fr       */
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
	modeHandlers['b'] = &Channel::handleModeBan;
}

Channel::~Channel(void) {}

/* PRIVATE METHODS */

void Channel::internalMessage(const Client &sender, const Client &client, const std::string &message) const {
	server.sendString(sender.getFd(), client.getFd(), message.c_str());
}

/* GETTERS */

const std::string&	Channel::getName(void) const { return (name); }
const std::string&	Channel::getTopic(void) const { return (topic); }
bool Channel::hasTopic(void) const { return (topic_set); }
bool Channel::isSecret(void) const { return (secret); }

/* SETTERS */

void	Channel::checkRights(const Client &executor, e_privlevel level) {
	if (level >= MEMBER && clients.find(&executor) == clients.end())
		throw (std::runtime_error("442 " + executor.getNick() + " "
				+ getName() + " :User not in the channel"));
	else if (level >= ADMIN && admins.find(&executor) == admins.end())
		throw std::runtime_error ("482 " + executor.getNick() + " "
				+ getName() + " :User is not an operator");
	else if (level == OWNER && &executor != owner)
		throw std::runtime_error ("482 " + executor.getNick() + " "
				+ getName() + " :User is not channel owner");
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
	channelMessage(admin.getFd(), getName() + " topic changed by "
			+ admin.getNick() + " to '"
			+ in_topic + "'");
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
	channelMessage(admin.getFd(), name + ": " + target.getNick()
			+ " has been granted admin by " + admin.getNick());
}

void	Channel::revokeAdmin(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (clients.find(&target) == clients.end())
		throw std::runtime_error(target.getNick() + " not in the channel!");
	if (admins.find(&target) == admins.end())
		throw (std::runtime_error("Target is not admin!"));
	admins.erase(&target);
	channelMessage(admin.getFd(), name + ": " + target.getNick()
			+ " admin rights revoked by " + admin.getNick());
}

void Channel::kickClient(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (clients.find(&target) == clients.end())
		throw (std::runtime_error(target.getNick() + " is not in channel"));
	if (owner != &admin && (admins.find(&target) != admins.end()))
		throw (std::runtime_error("Cannot kick admin"));
	channelMessage(admin.getFd(), name + ": " + target.getNick()
			+ " was kicked by " + admin.getNick());
	removeClient( target, "Kicked by " + admin.getNick() );
}

void Channel::banClient(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (clients.find(&target) != clients.end())
		kickClient(target, admin);
	if (invited_clients.find(&target) != invited_clients.end())
		revokeInvite(target, admin);
	banned_clients.insert(&target);  // Add the client to the invited list.
	channelMessage(server.getFd(), "MODE " + getName() + " +b " + target.getNick());
	internalMessage(admin, target, name + ": You have been banned by " + admin.getNick());
	internalMessage(admin, admin, name + ": You banned " + target.getNick());
}

void	Channel::revokeBan(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (banned_clients.find(&target) == banned_clients.end())
		throw (std::runtime_error(target.getNick() + " is not on ban list"));
	internalMessage(admin, target, name + ": Your ban has been revoked by " + admin.getNick());

	channelMessage(server.getFd(), "MODE " + getName() + " -b " + target.getNick());
	banned_clients.erase(&target);
}

/* JOIN - LEAVE */

void Channel::joinChannel(const Client &target, const std::string &password) {
	// Check if the channel is invite-only and if the client is invited.
	if (banned_clients.find(&target) != banned_clients.end())
		throw std::runtime_error("474 " + target.getNick()
				+ " " + getName() + " :User is banned from this channel!");
	if (clients.find(&target) != clients.end())
		throw std::runtime_error("443 " + target.getNick()
				+ " " + getName() + " :Already a channel member!");
	if (invite_only && invited_clients.find(&target) == invited_clients.end())
		throw std::runtime_error("473 " + target.getNick()
				+ " " + getName() + " :Channel is invite-only!");
	if (clients.size() >= clnts_limit) // If the channel is full, deny the client.
		throw std::runtime_error("471 " + target.getNick()
				+ " " + getName() + " :Channel is full and cannot accept members!");
	if (locked) { // If the channel isn't invite-only, check the password.
		if (hashed_pass != hashSimple(password)) {
			throw std::runtime_error("475 " + target.getNick()
				+ " " + getName() + " :Incorrect password");
		}
	}
	server.sendString(server.getFd(), target.getFd(), name + " :You have joined the channel");
	if (hasTopic())
		server.sendString(server.getFd(), target.getFd(),
				"332 " + target.getNick() + " " + getName() + " :" + getTopic());
	else
		server.sendString(server.getFd(), target.getFd(),
				"331 " + target.getNick() + " " + getName() + " :No topic is set");
	// Add the client to the channel.
	clients.insert(&target);
	channelMessage(server.getFd(), getName() + ": " + target.getNick() + " joined the channel!");
}

void Channel::removeClient(const Client &target, const std::string &reason) {
	channelMessage(target.getFd(), " PART " + getName() + " :" + reason);
	server.sendString(target.getFd(), target.getFd(), " PART " + getName() + " :" + reason);
	admins.erase(&target);  // Remove admin rights if applicable.
	clients.erase(&target); // Remove the client from the channel.

	// Reassign admin rights if the last admin leaves:
	if (admins.empty() && !clients.empty()) {
		admins.insert(*clients.begin());  // Assign the first client as the new admin
		channelMessage(server.getFd(), "MODE " + getName() + " +a " + (*clients.begin())->getNick());
	}
	if (owner == &target && !admins.empty()) { // if owner leaves, appoint another admin
		owner = *admins.begin();
		channelMessage(server.getFd(), getName() + " new channel owner: " + (*clients.begin())->getNick());
	}
	// If there are no clients left, you can clean up or close the channel.
	if (clients.empty()) {
		throw std::exception();
	}
}

/* GROUP MESSAGES */

void Channel::channelMessage(const int sender_fd, const std::string &message) {

	if (sender_fd != server.getFd())
		checkRights(server.getClientRef(sender_fd), MEMBER);
	for (std::set<const Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		// Don't send the message back to the sender
		if ((*it)->getFd() != sender_fd)
			server.sendString(sender_fd, (*it)->getFd(), message);
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
	internalMessage(admin, target, name + ": You have been invited to join by " + admin.getNick());
	server.sendString(server.getFd(), admin.getFd(), getName() + " :You invited " + target.getNick());
}

void	Channel::revokeInvite(const Client &target, const Client &admin) {
	checkRights(admin, ADMIN);
	if (invited_clients.find(&target) == invited_clients.end())
		throw (std::runtime_error(target.getNick() + " is not on invite list!"));
	invited_clients.erase(&target);
	internalMessage(admin, target, name + ": Your invite has been revoked by " + admin.getNick());
}

/* INFO */

bool Channel::containsMember(const Client &client ) const {
	if (clients.find(&client) != clients.end())
		return (true);
	return (false);
}
