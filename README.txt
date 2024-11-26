ft_irc project for school 42 with pyerima & mmakagon
Step one (penny)
1. server set up listening on specific port
2. client server connection

Hi! =)

I'm working on this list right now, I do everything in my fork:
https://github.com/makayun/16_ft_irc

IRCServer.cpp:

All channel logic should be in the Channel class, including admin rights checks (you can't kick someone if you're not an operator)
PRIVMSG sends a message to a channel for some reason, but it should send to an exact client
User should be authenticated only after setting a nick and a name
Client.hpp:

Channels are still are still public, will get back to it later, after fixing channels themself
Client.cpp:

Channels are still are still public, will get back to it later, after fixing channels themself
Channel.cpp:

Password is hashed now, but I have no idea how to ask users to enter it and how to check it, will work on it later
Need to implement MODE command
Overall:

We need to add somehow this ctrl+D thing (the subject, page 6), I have no idea how to do it
? When the server is closed - the user should be at least informed about it somehow, maybe?
Right now closing the server just cleans itself and that's it, but maybe I'm wrong.
I added closing of clients' fds when the programm closes (well, I believe I added, please check it in the IRCServer destructor),
maybe this should be placed in the client's destructor
Just a list of commands according to the RFC 1459 (an IRC standard):

KICK
Syntax:
KICK :[]
Forcibly removes from .[12] This command may only be issued by channel operators.

INVITE
Syntax:
INVITE
Invites to the channel .[9] does not have to exist, but if it does, only members of the channel are allowed to invite other clients. If the channel mode i is set, only channel operators may invite other clients.

TOPIC
Syntax:
TOPIC []
Allows the client to query or set the channel topic on .[43] If is given, it sets the channel topic to . If channel mode +t is set, only a channel operator may set the topic.

MODE
Syntax:
MODE (user)
MODE []
The MODE command is dual-purpose. It can be used to set both user and channel modes.
