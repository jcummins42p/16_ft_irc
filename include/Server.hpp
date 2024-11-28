/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024/11/27 14:41:51 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"
#include "ft_irc.hpp" // Ensure it includes the MAX_CLIENTS definition

#define BUFFER_SIZE 512 // Keep this here unless it's also defined elsewhere

class Server {
private:
    int server_fd;
    unsigned int hashed_pass;
    std::map<int, Client*> clients;
    std::map<std::string, Channel*> channels;
    std::ofstream logFile;

    void acceptClient(struct pollfd* fds);
    void handleClient(int client_fd);
    void processMessage(int client_fd, const std::string& message);

    // Command handlers
    void handleNickCommand(int client_fd, std::istringstream& iss);
    void handleUserCommand(int client_fd, std::istringstream& iss);
    void handleJoinCommand(int client_fd, std::istringstream& iss);
    void handlePartCommand(int client_fd, std::istringstream& iss);
    void handlePrivmsgCommand(int client_fd, std::istringstream& iss);
    void handleQuitCommand(int client_fd);
    void handleTopicCommand(int client_fd, std::istringstream& iss);
    void handleModeCommand(int client_fd, std::istringstream& iss);
    void handleKickCommand(int client_fd, std::istringstream& iss);
    void handleInviteCommand(int client_fd, std::istringstream& iss);

    void logEvent(const std::string& level, const std::string& message);
    std::string intToString(int number);

public:
    Server(int port, const std::string& in_pass);
    ~Server(void);
    void run();
};

#endif
