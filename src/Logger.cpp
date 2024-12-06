/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcummins <jcummins@student.42prague.c      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 17:43:14 by jcummins          #+#    #+#             */
/*   Updated: 2024/12/06 15:38:08 by jcummins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

Logger::Logger( void ) {
	logFile.open("server.log", std::ios::app);
}

Logger::~Logger( void ) {}

void Logger::info( const std::string &message) {
	output( timeStamp() + " [INFO] " + message );
}

void Logger::error( const std::string &message ) {
	output( timeStamp() + " [ERROR] " + message );
}

void Logger::warn( const std::string &message ) {
	output( timeStamp() + " [WARNING] " + message );
}

void Logger::output( const std::string &message ) {
	// Print to terminal
	//std::string logMessage = message;
	std::cout << message << std::endl;

	// Write to log file - change this to throw exception
	if (logFile.is_open()) {
		logFile << static_cast<std::string>(message) << std::endl;
	}
	else
		std::cout << "Log file not open, failed to write log\n";
}
