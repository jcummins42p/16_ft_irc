/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/18 13:42:36 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd) :
	fd(fd),
	is_autentificated(false) {}


Client::~Client( void ) {
	std::cout << "Reminder that you need to implement Client destructor" << std::endl;
}