#Server Process Introduction
This server process is designed to facilitate communication between clients over a network. It listens for incoming connections, handles message exchanges, and manages user registrations.

Getting Started
To compile and run the server process, follow these steps:

Compile Source Code:

cd chatserver
make all
Run the Server:

./server <portNumber> (e.g 7500)
Replace <portNumber> with the desired port number to listen on.

On another terminal:
nc 127.0.0.1 <portNumber>

protocals can be found in chatserver folder

Dependencies
Standard C Libraries:
ctype.h
sys/types.h
sys/socket.h
netinet/in.h
stdlib.h
arpa/inet.h
stdio.h
unistd.h
string.h
Functionality
The server process includes the following key functionalities:

Client Communication:

Sending and receiving messages between connected clients.
Handling message queues for clients when they are not immediately reachable.
User Registration:

Allowing clients to register usernames to identify themselves in the system.
Enforcing uniqueness of usernames.
Listing Connected Users:

Providing the ability to list all currently connected users.
Code Organization
The server source code is organized into the following files:

protocol.h: Header file containing protocol definitions.
libParseMessage.h: Header file for parsing incoming messages.
libMessageQueue.h: Header file for message queue management.
Functions
sendMessage(int sfd, char *toClient): Sends a single message to a client.
recvMessage(int sfd, char *fromClient): Reads a single message from a client.
max(int a, int b): Returns the maximum of two integers.
Error Handling
Error code 2: Indicates failure to send or receive data from the socket.
Error code 3: Indicates a message that exceeds the maximum allowed length.
Contributing
Contributions to 
