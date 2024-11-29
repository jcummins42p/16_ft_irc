/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 17:00:44 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/29 17:44:21 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include <sys/types.h>
#include "ft_irc.hpp"
#include <sys/socket.h>

/* CONSTRUCT - DESTRUCT */

Channel::Channel(const std::string& in_name, const Client& creator, const std::string& password)
    : clnts_limit(MAX_CLIENTS), invite_only(false), topic_admins_only(false) {
    if (in_name.empty())
        name = "Default chat";
    else
        name = in_name;
    
    if (password.empty())
        throw std::invalid_argument("Password must not be empty when creating a channel.");
    
    hashed_pass = hashSimple(password);  // Set the hashed password for the channel.
    clients.insert(&creator);           // Add the creator as the first client.
    admins.insert(&creator);            // Set the creator as the initial admin.
}

Channel::~Channel(void) {}


/* PRIVATE METHODS */

ssize_t Channel::internalMessage(const Client& client, const std::string message) const {
    return send(client.getFd(), message.c_str(), message.size(), 0);
}


/* GETTERS */

const std::string&	Channel::getName(void) const { return (name); }

const std::string&	Channel::getTopic(void) const { return (topic); }


/* SETTERS */

bool	Channel::setTopic(const std::string& in_topic, const Client& admin) {
	if (topic_admins_only && admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (in_topic.empty()) {
		internalMessage(admin, "Can't set an empty topic!\n");
		return (false);
	}
	else {
		topic = in_topic;
		return (true);
	}
}

bool	Channel::setPass(std::string& in_pass, const Client& admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (in_pass.empty()) {
		internalMessage(admin, "Can't set an empty password!\n");
		return (false);
	}
	else {
		hashed_pass = hashSimple(in_pass);
		return (true);
	}
}


/* ADMIN FUNCTIONS */

bool	Channel::addClient(const Client& in_client, const Client& admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (clients.size() >= clnts_limit) {
		internalMessage(admin, "Can't add a client - the channel is full!\n");
		return (false);
	}
	else {
		clients.insert(&in_client);
		return (true);
	}
}

bool	Channel::addAdmin(const Client& in_client, const Client& admin) {
	if (admins.find(&admin) == admins.end()) {
		internalMessage(admin, "You don't have admin rights!\n");
		return (false);
	}
	else if (clients.find(&in_client) == clients.end()) {
		internalMessage(admin, "Can't make an admin - the client is not in the channel!\n");
		return (false);
	}
	else {
		admins.insert(&in_client);
		return (true);
	}
}

bool Channel::kickClient(const Client& in_client, const Client& admin) {
    if (admins.find(&admin) == admins.end()) {
        internalMessage(admin, "You don't have admin rights!\n");
        return false;
    }
    else if (clients.find(&in_client) == clients.end()) {
        internalMessage(admin, "Can't kick - the user is not in the channel!\n");
        return false;
    }
    else if (admins.find(&in_client) != admins.end()) {
        internalMessage(admin, "You cannot kick an admin!\n");
        return false;  // Ensure admins cannot be kicked by non-admins
    }
    else {
        clients.erase(&in_client);  // Remove the user from the channel
        return true;
    }
}


bool Channel::kickAdmin(const Client& in_client, const Client& admin) {
    if (admins.find(&admin) == admins.end()) {
        internalMessage(admin, "You don't have admin rights!\n");
        return false;
    }
    else if (clients.find(&in_client) == clients.end()) {
        internalMessage(admin, "Can't kick - the user is not in the channel!\n");
        return false;
    }
    else if (admins.find(&in_client) == admins.end()) {
        internalMessage(admin, "Can't kick - the user is not an admin!\n");
        return false;
    }
    else {
        admins.erase(&in_client);  // Remove the user from the admin list

        // If all admins are removed, assign a new admin if there are clients left.
        if (admins.empty() && !clients.empty()) {
            // Assign the first client as the new admin
            admins.insert(*clients.begin());
            internalMessage(**clients.begin(), "You are now the channel admin.\n");
        }

        return true;
    }
}



/* JOIN - LEAVE */

bool Channel::joinChannel(const Client& in_client, const std::string& password) {
    // Check if the channel is invite-only and if the client is invited.
    if (invite_only && invited_clients.find(&in_client) == invited_clients.end()) {
        internalMessage(in_client, "Sorry, the chat is invite-only!\n");
        return false;
    }

    // If the channel isn't invite-only, check the password.
    if (!invite_only) {
        unsigned int hashed_attempt = hashSimple(password);
        if (hashed_attempt != hashed_pass) {
            internalMessage(in_client, "Incorrect password!\n");
            return false;
        }
    }

    // If the channel is full, deny the client.
    if (clients.size() >= clnts_limit) {
        internalMessage(in_client, "Sorry, the chat is full!\n");
        return false;
    }

    // Add the client to the channel.
    clients.insert(&in_client);
    return true;
}

// // Function to create a new channel
// void Channel::create(const Client& creator, const std::string& password) {
//     // This function initializes the channel with a password and sets the creator as the admin
//     hashed_pass = hashSimple(password);  // Store hashed password
//     clients.insert(&creator);            // Add the creator to the channel
//     admins.insert(&creator);             // Set creator as an admin
// }


bool Channel::leaveChannel(const Client& in_client) {
    admins.erase(&in_client);  // Remove admin rights if applicable.
    clients.erase(&in_client); // Remove the client from the channel.

    // Reassign admin rights if the last admin leaves:
    if (admins.empty() && !clients.empty()) {
        admins.insert(*clients.begin());  // Assign the first client as the new admin
        internalMessage(**clients.begin(), "You are now the channel admin.\n");  // Correctly pass the reference
    }
    
    // If there are no clients left, you can clean up or close the channel.
    if (clients.empty()) {
        // Clean up the channel or leave it open for future clients (optional logic)
        // Example: delete the channel, or make it inactive
    }

    return true;
}



void Channel::create(const Client& creator, const std::string& password) {
    hashed_pass = hashSimple(password);  // Set hashed password for the channel
    clients.insert(&creator);            // Add the creator to the clients
    admins.insert(&creator);             // Set creator as the admin
    // Other initialization logic if needed...
}

/* GROUP MESSAGES */

void Channel::channelMessage(const std::string message, const Client& sender) const {
    for (std::set<const Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        // Don't send the message back to the sender
        if (*it != &sender) {
            send((*it)->getFd(), message.c_str(), message.size(), 0);
        }
    }
}


/* INVITE MANAGEMENT */

void Channel::inviteClient(const Client& in_client, const Client& admin) {
    if (admins.find(&admin) == admins.end()) {
        internalMessage(admin, "You don't have admin rights!\n");
        return;
    }
    
    invited_clients.insert(&in_client);  // Add the client to the invited list.
    internalMessage(in_client, "You have been invited to join the channel: " + name + "\n");
}
