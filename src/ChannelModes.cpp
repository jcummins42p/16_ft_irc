/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelModes.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:43:14 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/13 16:35:16 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

std::string Channel::modeHandler(int client_fd, std::istringstream &iss)
{
	std::string mode, input, output;
	char command;
	bool toggle = false;
	iss >> mode;

	if (mode.size() != 2)
		throw std::runtime_error("Invalid mode switch '" + mode + "'");
	if (mode[0] == '+')
		toggle = true;
	else if (mode[0] != '-')
		throw std::runtime_error("Mode switch must start with '-' or '+");
	std::getline(iss, input);
	colonectomy(input);
	command = mode[1];
	std::map<char, ChanModeHandler>::iterator it = modeHandlers.find(command);
	if (it != modeHandlers.end()) {
		output = (this->*(it->second))(client_fd, input, toggle);
	} else {
		throw std::runtime_error("Invalid mode switch '" + mode + "'");
	}
	return (output);
}

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

std::string Channel::handleModeOperator(int client_fd, const std::string &input, bool toggle)  {
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
	return ("");
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

std::string Channel::handleModeSecret(int client_fd, const std::string &input, bool toggle) {
	std::string message = getName() + ": secret mode ";
	(void) client_fd;
	(void) input;

	message += (toggle ? "ON" : "OFF");
	secret = toggle;
	return (message);
}