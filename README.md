# Overall Introduction:
This is an individual assignment in utm csc209, my job was to implement the chatserver.c file in charserver folder. The onwer of the course has the copyright.

## Server Process Introduction
This server process is designed to facilitate communication between clients over a network on tcp protocal, but this project only focous on the communication on localhost. It listens for incoming connections, handles message exchanges, and manages user registrations.

### Getting Started

#### To compile and run the server process, follow these steps:
```bash
git clone https://github.com/DavisDongMZ/chatserver-in-C
cd chatserver
make all
```

#### Run the Server:
```bash
./chatserver <portNumber> (e.g 7500)
```
Replace <portNumber> with the desired port number to listen on.

#### On another terminal(s):
```bash
nc 127.0.0.1 <portNumber>
```

protocals can be found in chatserver folder

### Dependencies on Standard C Libraries: 
- ctype.h
- sys/types.h
- sys/socket.h
- netinet/in.h
- stdlib.h
- arpa/inet.h
- stdio.h
- unistd.h
- string.h
- Functionality

### The server process includes the following key functionalities:

#### Client Communication:

Sending and receiving messages between connected clients.
Handling message queues for clients when they are not immediately reachable.

#### User Registration:

Allowing clients to register usernames to identify themselves in the system.
Enforcing uniqueness of usernames.

#### Listing Connected Users:

Providing the ability to list all currently connected users.

### Code Organization
The server source code is organized into the following files:

- protocol.h: Header file containing protocol definitions.
- libParseMessage.h: Header file for parsing incoming messages.
- libMessageQueue.h: Header file for message queue management.

### Functions
- sendMessage(int sfd, char *toClient): Sends a single message to a client.
- recvMessage(int sfd, char *fromClient): Reads a single message from a client.
- max(int a, int b): Returns the maximum of two integers.
- main(): handle the server.

### Error Handling
- Error code 2: Indicates failure to send or receive data from the socket.
- Error code 3: Indicates a message that exceeds the maximum allowed length.

## Contributing
You are welcome to contribute to this project, I'll be appreciate if you can make and enhancement on this project :).

### Difficulties on this assignment
I found this assignment had some difficulties as:
- Parse the messages, when it's too long it will smash the stack. Hence I need to deal with some bad input and avoid that happen.
- Multiple clients, this server support at most 32 different clients. I need to understand how socket works in C and how file descriptors works in C, because each client it's represented by one fire descriptor.
- Understand how select() works in C for efficiently managing multiple I/O operations within a single process. Here if I use the fork(), this server can't handle the stuck.
 
