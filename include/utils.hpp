/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:44:19 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/18 18:11:45 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
# define UTILS_H

# include <iostream>
# include <cstdio>
# include <string>
# include <sstream>


std::string 	colonectomy( std::string &msg );
unsigned int	hashSimple( const std::string &in_str );
std::string		intToString( int number );
std::string		timeStamp( void );
bool			caseInsCompare( const std::string &a, const std::string &b) ;

#endif

