/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hashSimple.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 11:29:40 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/27 14:53:14 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"

unsigned int	hashSimple(const std::string& in_str) {
	std::string str_copy = in_str;  // Create a local copy
    unsigned int hashed = 0;

	for (size_t i = 0; i < str_copy.length(); ++i) {
        str_copy[i] = std::tolower(str_copy[i]);  // Example: modifying the copy
        hashed = hashed * 31 + str_copy[i];
    }
	return (hashed);
}
