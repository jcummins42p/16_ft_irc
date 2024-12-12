/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelModes.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:43:14 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/12 22:40:14 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

std::string Channel::handleModeInvite(int client_fd, const std::string &input, bool toggle) {
	std::string message = getName() + ": invite mode ";
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

std::string Channel::handleModeTopic(int client_fd, const std::string &input, bool toggle)  {
	std::string message = getName() + ": topic lock ";

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

std::string Channel::handleModeKey(int client_fd, const std::string &input, bool toggle) {
	std::string message = getName() + ": password mode ";
	if (toggle) {
		message += "ON";
		setPass(input, server.getClientRef(client_fd));
		message += ", password set to: " + input;
	}
	else
		message += "OFF";
	locked = toggle;
	return (message);
}

void Channel::handleModeOperator(int client_fd, const std::string &input, bool toggle)  {
	std::istringstream iss(input);
	std::string	target;
	int	n_valid = 0;

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
			n_valid++;
		}
		catch (std::exception &e) {
			server.sendString(client_fd, std::string(e.what()));
		}
	}
	if (n_valid == 0)
		throw std::runtime_error("Provide at least one valid client");
}

std::string Channel::handleModeUserLimit(int client_fd, const std::string &input, bool toggle)  {
	std::string message = getName() + ": user limit mode ";

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

std::string Channel::handleModeSecret(bool toggle) {
	std::string message = getName() + ": secret mode ";

	message += (toggle ? "ON" : "OFF");
	secret = toggle;
	return (message);
}
