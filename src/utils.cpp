/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:43:14 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/06 19:13:26 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

unsigned int hashSimple(const std::string& in_str) {
	std::string str_copy = in_str;  // Create a local copy
	unsigned int hashed = 0;

	for (size_t i = 0; i < str_copy.length(); ++i) {
		str_copy[i] = std::tolower(str_copy[i]);  // Example: modifying the copy
		hashed = hashed * 31 + str_copy[i];
	}
	return (hashed);
}

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

bool	caseInsCompare( const std::string &a, const std::string &b) {
	unsigned long size = a.size();
	if (size != b.size())
		return (false);
	for (unsigned long i = 0; i < size; i++) {
		if (std::tolower(a[i]) != std::tolower(b[i]))
			return (false);
	}
	return (true);
}
