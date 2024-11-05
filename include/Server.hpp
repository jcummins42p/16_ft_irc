/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-05 14:46:01 by pyerima           #+#    #+#             */
/*   Updated: 2024-11-05 14:46:01 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include <netinet/in.h>

class Client;

class Server {
public:
    Server(int port);
    ~Server();
    
    bool start();
    void run();

private:
    int port_;
    int server_fd_;
    std::map<int, Client*> clients_;
    std::map<std::string, std::set<int> > channels_;

    void acceptClient();
    void disconnectClient(int fd);
    void handleMessage(int fd, const std::string& message);

    //command handlers
    void handleNick(int fd, const std::string& nickname);
    void handleJoin(int fd, const std::string& channel);
    void handlePrivMsg(int fd, const std::string& recipient, const std::string& message);
};

class Client {
public:
    Client(int fd);
    ~Client();

    void setNickname(const std::string& nickname);
    std::string getNickname() const;
    int getFd() const;

private:
    int fd_;
    std::string nickname_;
};

#endif
