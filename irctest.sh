#!/bin/bash

SERVER="localhost"
PORT="6626"

send_command() {
    echo -e "$1" | nc $SERVER $PORT
}

echo "connecting clients..."

echo "connecting client1 and joining #testchannel..."
(send_command "NICK client1\r\nUSER client1 0 * :Client One\r\nJOIN #testchannel\r\n" &)
sleep 1

echo "connecting client2 and joining #testchannel..."
(send_command "NICK client2\r\nUSER client2 0 * :Client Two\r\nJOIN #testchannel\r\n" &)
sleep 1

echo "setting client1 as the operator of #testchannel..."
send_command "MODE #testchannel +o client1\r\n"
sleep 1

echo "inviting client3 to #testchannel by client1..."
send_command "NICK client1\r\nINVITE client3 #testchannel\r\n"
sleep 1

echo "connecting client3 and joining #testchannel..."
(send_command "NICK client3\r\nUSER client3 0 * :Client Three\r\nJOIN #testchannel\r\n" &)
sleep 1

echo "kicking client2 from #testchannel by client1..."
send_command "KICK #testchannel client2 :Bye\r\n"
sleep 1

echo "removing operator privilege from client1..."
send_command "MODE #testchannel -o client1\r\n"
sleep 1

echo "sending a message from client1 in #testchannel..."
send_command "PRIVMSG #testchannel :Hello, everyone!\r\n"
sleep 1

echo "all tests completed."
