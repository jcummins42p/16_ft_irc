/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:43:14 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/06 10:06:18 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::string timeStamp( void ) {
	time_t now = time(0);
	struct tm *localTime = localtime(&now);

	char buf[30];
	strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", localTime);
	return (std::string (buf));
}

std::string intToString( int number ) {
	std::ostringstream oss;
	oss << number;
	return oss.str();
}
