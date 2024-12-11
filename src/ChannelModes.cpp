/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelModes.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:43:14 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/11 17:22:54 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

std::string Channel::handleModeInvite(int client_fd, std::string &input, bool toggle) {
	(void) input;
	(void) toggle;
	(void) client_fd;
	std::string message = "Turning " + getName() + " invite mode ";

	return (message);
}

std::string Channel::handleModeTopic(int client_fd, std::string &input, bool toggle)  {
	(void) input;
	(void) client_fd;
	(void) toggle;
	std::string message = "Turning " + getName() + " topic lock ";

	return (message);
}

std::string Channel::handleModeKey(int client_fd, std::string &input, bool toggle) {
	std::string message = "Turning " + getName() + " password mode ";
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

std::string Channel::handleModeOperator(int client_fd, std::string &input, bool toggle)  {
	(void) input;
	(void) client_fd;
	std::string message = (toggle ? "Granting" : "Revoking");
	message += input + "operator rights on " + getName();
	return (message);
}

std::string Channel::handleModeUserLimit(int client_fd, std::string &input, bool toggle)  {
	(void) input;
	(void) client_fd;
	std::string message = "Turning " + getName() + " limit mode ";

	message += (toggle ? "ON" : "OFF" );
	if (toggle)
		message += ": limit now " + input;
	return (message);
}

