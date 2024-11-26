/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simpleHash.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmakagon <mmakagon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 11:29:40 by mmakagon          #+#    #+#             */
/*   Updated: 2024/11/19 13:22:35 by mmakagon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"

unsigned int	simpleHash(std::string& in_str) {
	unsigned int	hashed = 0;

	for(size_t i = 0; i < in_str.size(); ++i) {
		hashed += (i * i + in_str.size()) * static_cast<unsigned int>(in_str[i]);
		in_str[i] = 0;
	}
	return (hashed);
}
