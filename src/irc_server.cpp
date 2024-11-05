/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc_server.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pyerima <pyerima@student.42.fr>            #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-11-05 14:48:02 by pyerima           #+#    #+#             */
/*   Updated: 2024-11-05 14:48:02 by pyerima          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdlib.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 512

class Client {
public:
    int fd;
    std::string nick;
    std::string user;
    std::set<std::string> channels;
    bool is_operator;

    Client(int fd) : fd(fd), is_operator(false) {}
};

class Channel {
public:
    std::string name;
    std::set<int> clients; //client file descriptors
    std::string topic;
    std::string password;
    bool invite_only;
    bool topic_restricted;
    int user_limit;

    Channel(const std::string& name) 
        : name(name), invite_only(false), topic_restricted(false), user_limit(-1) {}

    void join(int client_fd) {
        clients.insert(client_fd);
    }

    void part(int client_fd) {
        clients.erase(client_fd);
    }

    void sendMessage(const std::string& message, int sender_fd) {
        //for loop to iterate through the set of clients
        for (std::set<int>::iterator it = clients.begin(); it != clients.end(); ++it) {
            int client_fd = *it;
            if (client_fd != sender_fd) {
                send(client_fd, message.c_str(), message.length(), 0);
            }
        }
    }

    void setTopic(const std::string& new_topic) {
        topic = new_topic;
    }

    std::string getTopic() const {
        return topic;
    }

    void setPassword(const std::string& pass) {
        password = pass;
    }

    bool isOperator(int client_fd) {
        return clients.find(client_fd) != clients.end(); // Simplified for demonstration
    }
};

class IRCServer {
private:
    int server_fd;
    std::map<int, Client*> clients;
    std::map<std::string, Channel*> channels;

public:
    IRCServer(int port) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        listen(server_fd, 5);
    }

    ~IRCServer() {
        //for loop to iterate through the clients map
        for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
            delete it->second;
        }
        //for loop to iterate through the channels map
        for (std::map<std::string, Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
            delete it->second;
        }
        close(server_fd);
    }

    void run() {
        struct pollfd fds[MAX_CLIENTS + 1];
        fds[0].fd = server_fd;
        fds[0].events = POLLIN;

        for (int i = 1; i <= MAX_CLIENTS; i++) {
            fds[i].fd = -1;
        }

        while (true) {
            // int ret = poll(fds, MAX_CLIENTS + 1, -1);
            poll(fds, MAX_CLIENTS + 1, -1);
            for (int i = 0; i <= MAX_CLIENTS; i++) {
                if (fds[i].revents & POLLIN) {
                    if (fds[i].fd == server_fd) {
                        acceptClient(fds);
                    } else {
                        handleClient(fds[i].fd);
                    }
                }
            }
        }
    }

private:
    void acceptClient(struct pollfd* fds) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            std::cerr << "Failed to accept client" << std::endl;
            return;
        }

        std::cout << "Client connected: " << client_fd << std::endl;
        clients[client_fd] = new Client(client_fd);

        // add new client to poll array
        for (int i = 1; i <= MAX_CLIENTS; i++) {
            if (fds[i].fd == -1) {
                fds[i].fd = client_fd;
                fds[i].events = POLLIN;
                break;
            }
        }
    }
    //handle messages from a client
    void handleClient(int client_fd) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Client disconnected: " << client_fd << std::endl;
            } else {
                std::cerr << "Receive error on client: " << client_fd << std::endl;
            }
            close(client_fd);
            clients.erase(client_fd);
            return;
        }

        buffer[bytes_received] = '\0';
        std::string message(buffer);
        processMessage(client_fd, message);
    }

    void processMessage(int client_fd, const std::string& message) {
        std::istringstream iss(message);
        std::string command;
        iss >> command;

        if (command == "NICK") {
            handleNickCommand(client_fd, iss);
        } else if (command == "USER") {
            handleUserCommand(client_fd, iss);
        } else if (command == "JOIN") {
            handleJoinCommand(client_fd, iss);
        } else if (command == "PART") {
            handlePartCommand(client_fd, iss);
        } else if (command == "PRIVMSG") {
            handlePrivmsgCommand(client_fd, iss);
        } else if (command == "QUIT") {
            handleQuitCommand(client_fd);
        } else if (command == "PING") {
            send(client_fd, "PONG :ping\n", 12, 0);
        } else if (command == "TOPIC") {
            handleTopicCommand(client_fd, iss);
        } else if (command == "MODE") {
            handleModeCommand(client_fd, iss);
        } else if (command == "KICK") {
            handleKickCommand(client_fd, iss);
        } else if (command == "INVITE") {
            handleInviteCommand(client_fd, iss);
        }
    }

    //the NICK command
    void handleNickCommand(int client_fd, std::istringstream& iss) {
        std::string nick;
        iss >> nick; // Read the nickname from the stream
        clients[client_fd]->nick = nick; // Set the client's nickname
        std::cout << "Client " << client_fd << " set nickname to " << nick << std::endl;
    }

    //the USER command
    void handleUserCommand(int client_fd, std::istringstream& iss) {
        std::string user;
        iss >> user; // Read the username from the stream
        clients[client_fd]->user = user; // Set the client's username
        std::cout << "Client " << client_fd << " set username to " << user << std::endl;
    }

    //the JOIN command
    void handleJoinCommand(int client_fd, std::istringstream& iss) {
        std::string channel_name;
        iss >> channel_name; // read the channel name from the stream
        Channel* channel;

        // check if the channel exists
        if (channels.find(channel_name) == channels.end()) {
            channel = new Channel(channel_name); // create a new channel if it doesn't exist
            channels[channel_name] = channel; // add channel to the map
        } else {
            channel = channels[channel_name]; // get existing channel
        }

        channel->join(client_fd); // add client to the channel
        clients[client_fd]->channels.insert(channel_name); // update clients channel list
        std::cout << "Client " << client_fd << " joined channel " << channel_name << std::endl;
    }

    //the PART command
    void handlePartCommand(int client_fd, std::istringstream& iss) {
        std::string channel_name;
        iss >> channel_name;

        if (channels.find(channel_name) != channels.end()) {
            channels[channel_name]->part(client_fd);
            clients[client_fd]->channels.erase(channel_name);
            send(client_fd, ("Left channel: " + channel_name + "\n").c_str(), 25, 0);
            channels[channel_name]->sendMessage(clients[client_fd]->nick + " has left the channel.\n", client_fd);
        } else {
            send(client_fd, "Channel not found.\n", 20, 0);
        }
    }

    //the PRIVMSG command
    void handlePrivmsgCommand(int client_fd, std::istringstream& iss) {
        std::string target;
        std::string msg;
        iss >> target;
        std::getline(iss, msg);

        if (channels.find(target) != channels.end()) {
            channels[target]->sendMessage(clients[client_fd]->nick + ": " + msg + "\n", client_fd);
        } else {
            send(client_fd, "Target not found.\n", 18, 0);
        }
    }

    //the QUIT command
    void handleQuitCommand(int client_fd) {
        std::cout << "Client " << client_fd << " disconnected." << std::endl;
        close(client_fd); // Close the client's socket
        clients.erase(client_fd); // Remove client from the map
    }

    //the TOPIC command
    void handleTopicCommand(int client_fd, std::istringstream& iss) {
        std::string channel_name, new_topic;
        iss >> channel_name; // read channel name
        std::getline(iss, new_topic); // read the new topic

        if (channels.find(channel_name) != channels.end()) {
            Channel* channel = channels[channel_name]; // get channel
            channel->setTopic(new_topic); // set the topic
            channel->sendMessage("TOPIC " + channel_name + " : " + new_topic + "\n", client_fd); // notify clients of the new topic
        } else {
            send(client_fd, "No such channel.\n", 18, 0); // notify client of error
        }
    }



    //the MODE command
    void handleModeCommand(int client_fd, std::istringstream& iss) {
        std::string channel_name, mode;
        iss >> channel_name >> mode; // read channel name and mode
        //setting modes
        std::cout << "Client " << client_fd << " set mode for channel " << channel_name << " to " << mode << std::endl;
    }

    //the KICK command
    void handleKickCommand(int client_fd, std::istringstream& iss) {
        std::string channel_name, target_nick;
        iss >> channel_name >> target_nick; // read channel name and target nickname
        //
        std::cout << "Client " << client_fd << " kicked " << target_nick << " from channel " << channel_name << std::endl;
    }

    //the INVITE command
    void handleInviteCommand(int client_fd, std::istringstream& iss) {
        std::string target_nick, channel_name;
        iss >> target_nick >> channel_name; // Read target nickname and channel name
        // Simplified handling
        std::cout << "Client " << client_fd << " invited " << target_nick << " to channel " << channel_name << std::endl;
    }
};

// Main function to start the server
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    IRCServer server(port);
    server.run();

    return 0;
}
