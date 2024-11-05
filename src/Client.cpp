/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/05 18:28:58 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd) :
	fd(fd),
	is_operator(false) {}

Client::~Client( void ) {
	std::cout << "Reminder that you need to implement Client destructor" << std::endl;
}
