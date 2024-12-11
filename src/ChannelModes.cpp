/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelModes.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:43:14 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/11 21:39:25 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

// This is working
std::string Channel::handleModeInvite(int client_fd, const std::string &input, bool toggle) {
	std::string message = getName() + " invite mode ";
	std::istringstream iss(input);
	std::string	invite;

	if (toggle)
		message += "ON";
	else
		message += "OFF";
	invite_only = toggle;
	while (iss >> invite)
	{
		try {
			inviteClient(server.getClientRef(invite), server.getClientRef(client_fd));
			server.sendString(client_fd, "Invited " + invite + " to " + getName());
		}
		catch (std::exception &e) {
			server.sendString(client_fd, std::string(e.what()));
		}
	}
	return (message);
}

//	This is working
std::string Channel::handleModeTopic(int client_fd, const std::string &input, bool toggle)  {
	std::string message = getName() + " topic lock ";

	if (toggle)
		message += "ON";
	else
		message += "OFF";
	topic_admins_only = toggle;
	if (input.size() > 0) {
		setTopic(input, server.getClientRef(client_fd));
		message += ", topic set to: " + input;
	}
	return (message);
}

//	This is working
std::string Channel::handleModeKey(int client_fd, const std::string &input, bool toggle) {
	std::string message = getName() + " password mode ";
	if (toggle) {
		message += "ON";
		setPass(input, server.getClientRef(client_fd));
		message += ", password set to: " + input;
	}
	else
		message += "OFF";
	pass_required = toggle;
	return (message);
}

//	Works but sends too many messages
void Channel::handleModeOperator(int client_fd, const std::string &input, bool toggle)  {
	std::istringstream iss(input);
	std::string	target;

	while (iss >> target)
	{
		try {
			if (toggle) {
				addAdmin(server.getClientRef(target), server.getClientRef(client_fd));
				server.sendString(client_fd, "Added " + target + " to " + getName() + " admin");
			} else {
				revokeAdmin(server.getClientRef(target), server.getClientRef(client_fd));
				server.sendString(client_fd, "Removed " + target + " from " + getName() + " admin");
			}
		}
		catch (std::exception &e) {
			server.sendString(client_fd, std::string(e.what()));
		}
	}
}

//	Working
std::string Channel::handleModeUserLimit(int client_fd, const std::string &input, bool toggle)  {
	std::string message = getName() + " user limit mode ";

	if (toggle) {
		message += "ON";
		std::cout << "Trying to convert to nubmer: '" << input << "'";
		setUserLimit(std::atol(input.c_str()), server.getClientRef(client_fd));
		message += ", limit set to: " + input;
	}
	else {
		message += "OFF";
		clnts_limit = SIZE_MAX;
	}
	return (message);
}
