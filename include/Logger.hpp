/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:44:19 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/06 10:08:59 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_H
# define LOGGER_H

# include "utils.hpp"
# include <iostream>
# include <sstream>
# include <fstream>

class	Logger
{
	public:
		Logger	( void );
		~Logger	( void );

		// cannot be const because writing to the ofstream is considered nonconst
		void info(const std::string &message)  ;
		void error(const std::string &message)  ;
		void warn(const std::string &message)  ;
	private:
		Logger	( const Logger &other );
		Logger	&operator=( const Logger &other );

		std::ofstream logFile;
		void	output( const std::string &message ) ;
} ;

#endif

