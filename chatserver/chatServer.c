/* server process */

/* include the necessary header files */
#include<ctype.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>

#include "protocol.h"
#include "libParseMessage.h"
#include "libMessageQueue.h"

int max(int a, int b) {
    if (a>b)return a;
    return b;
}

struct client {
    char user[MAX_USER_LEN+1];
	char touser[MAX_USER_LEN+1];
    int fd;
    char buf[MAX_CHAT_MESSAGE_LEN];
    int inbuf;
    MessageQueue queue;
};
/**
 * send a single message to client
 * sockfd: the socket to read from
 * toClient: a buffer containing a null terminated string with length at most
 * 	     MAX_MESSAGE_LEN-1 characters. We send the message with \n replacing \0
 * 	     for a mximmum message sent of length MAX_MESSAGE_LEN (including \n).
 * return 1, if we have successfully sent the message
 * return 2, if we could not write the message
 */
int sendMessage(int sfd, char *toClient){
    char message[MAX_MESSAGE_LEN];
    snprintf(message, MAX_MESSAGE_LEN, "%s\n", toClient);

    int len = strlen(message);
    int total_sent = 0;
    int sent;

    while (total_sent < len) {
        sent = send(sfd, message + total_sent, len - total_sent, 0);
        if (sent < 0) {
            return 2; // Error: send failed
        }
        total_sent += sent;
    }
    return 1;
}

/**
 * read a single message from the client.
 * sockfd: the socket to read from
 * fromClient: a buffer of MAX_MESSAGE_LEN characters to place the resulting message
 *             the message is converted from newline to null terminated,
 *             that is the trailing \n is replaced with \0
 * return 1, if we have received a newline terminated string
 * return 2, if the socket closed (read returned 0 characters)
 * return 3, if we have read more bytes than allowed for a message by the protocol
 */
int recvMessage(int sfd, char *fromClient){
	char buff[MAX_MESSAGE_LEN];
	int len = 0;
	int total_len = 0;

	while (total_len < MAX_MESSAGE_LEN && buff[len-1] != '\n') {
        	len = recv(sfd, buff + total_len, MAX_MESSAGE_LEN - total_len, 0);
        	if (len <= 0) {
        		return 2; // Error: no data received
        	}
                total_len += len;
	}

	if (total_len == MAX_MESSAGE_LEN && buff[MAX_MESSAGE_LEN-1] != '\n') {
        	return 3; // Error: message too long
        }

    	// Replace newline with null terminator
    	if (buff[len-1] == '\n') {
    	    buff[len-1] = '\0';
    	} else {
    	    buff[len] = '\0';
    	}
	strcpy(fromClient, buff);
    	return 1;
}

int main (int argc, char ** argv) {
    int sockfd;

    if(argc!=2){
	    fprintf(stderr, "Usage: %s portNumber\n", argv[0]);
	    exit(1);
    }
    int port = atoi(argv[1]);

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("socket call failed");
        exit (1);
    }

    struct sockaddr_in server;
    server.sin_family=AF_INET;          // IPv4 address
    server.sin_addr.s_addr=INADDR_ANY;  // Allow use of any interface 
    server.sin_port = htons(port);      // specify port

    if (bind (sockfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror ("bind call failed");
        exit (1);
    }

    if (listen (sockfd, 5) == -1) {
        perror ("listen call failed");
        exit (1);
    }

    int fdlist[24];
    int fdcount=0;
    fdlist[fdcount++]=sockfd;
    struct client clientList[32];

    for (;;) {
	fd_set readfds, writefds, exceptfds;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);

        int fdmax=0;
        for (int i=0; i<fdcount; i++) {
            if (fdlist[i]>0) {
                FD_SET(fdlist[i], &readfds);
		FD_SET(fdlist[i], &writefds);
                fdmax=max(fdmax,fdlist[i]);
            }
        }

        struct timeval tv;
        tv.tv_sec=5;
        tv.tv_usec=0;

	int numfds;
	if ((numfds=select(fdmax+1, &readfds, &writefds, &exceptfds, &tv))>0) { // block!!
	for (int i=0; i<fdcount; i++) {
                if (FD_ISSET(fdlist[i],&readfds)) {
                   if (fdlist[i]==sockfd && fdcount<32) { /*accept a connection */
                        int newsockfd;
                        if ((newsockfd = accept (sockfd, NULL, NULL)) == -1) {
                            perror ("accept call failed");
                            continue;
                        }
                        fdlist[fdcount]=newsockfd;
			strcpy(clientList[fdcount].user, "");
    			clientList[fdcount].fd = newsockfd;
    			strcpy(clientList[fdcount].buf, "");
    			clientList[fdcount].inbuf = 0;
    			initQueue(&clientList[fdcount].queue);
			fdcount++;
                    } else { /* read from an existing connection: guaranteed 1 non-blocking read */
                        char fromClient[MAX_MESSAGE_LEN], toClient[MAX_MESSAGE_LEN];
                        int retVal=recvMessage(fdlist[i], fromClient);
                        if (retVal==1) {
				char *part[4];
				int numParts=parseMessage(fromClient, part);
				if(numParts==0){
					strcpy(toClient,"ERROR");
					sendMessage(fdlist[i], toClient);
				}else if(strcmp(part[0], "list")==0){
					strcpy(toClient, "users: ");
					for (int k = 1; k < fdcount; k++) {
						if(k==fdcount-1||k==10){strcat(toClient, clientList[k].user);break;}
        					strcat(toClient, clientList[k].user);
        					strcat(toClient, ", ");
    					}
					sendMessage(fdlist[i], toClient);
				}else if(strcmp(part[0], "quit")==0){
					strcpy(toClient, "closing");
					sendMessage(fdlist[i], toClient);
            				close (fdlist[i]);
					if (i != fdcount-1) {
                				fdlist[i] = fdlist[fdcount-1];
						clientList[i]=clientList[fdcount-1];
            				}
					fdcount--;
				} else if(strcmp(part[0], "getMessage")==0){
					if(dequeue(&clientList[i].queue, toClient)){
						sendMessage(fdlist[i], toClient);
					} else {
						strcpy(toClient, "noMessage");
						sendMessage(fdlist[i], toClient);
					}
				}else if(strcmp(part[0], "message")==0){
					char *fromUser=part[1];
					char *toUser=part[2];
					char *message=part[3];
					if(strcmp(fromUser, clientList[i].user)!=0){
							sprintf(toClient, "invalidFromUser:%s",fromUser);
							sendMessage(fdlist[i], toClient);
							continue;
					}for(int start=0; start < fdcount; start++){
						if(strcmp(toUser, clientList[start].user)==0){
							sprintf(clientList[i].buf, "%s:%s:%s:%s","message", fromUser, toUser, message);
							sprintf(clientList[i].touser, "%s", toUser);
							break;
						}
						if(start==fdcount-1){
							sprintf(toClient, "invalidToUser:%s",toUser);
			                                sendMessage(fdlist[i], toClient);
						}
					}
				} else if(strcmp(part[0], "register")==0){
					for(int j=1; j<fdcount; j++){
						if(j!=i){
							if(strncmp(clientList[j].user, part[1], MAX_USER_LEN)==0){
								strcpy(toClient, "ERROR");
								sendMessage(fdlist[i], toClient);
								break;
							}
						}else{
							if(strncmp(clientList[i].user, part[1], MAX_USER_LEN)!=0){
								strcpy(clientList[i].user, part[1]);
								strcpy(toClient, "registered");
								sendMessage(fdlist[i], toClient);
							} else {
								strcpy(toClient, "ERROR");
								sendMessage(fdlist[i], toClient);
							}
						}
					}
				}
	    		}else {
            			close (fdlist[i]);
	    		}
                    }
                }else if(FD_ISSET(fdlist[i],&writefds)&&strlen(clientList[i].buf)!=0){
			char  toClient[MAX_MESSAGE_LEN];
			sprintf(toClient, "%s", clientList[i].buf);
			for(int start=1; start < fdcount; start++){
			if(strcmp(clientList[i].touser, clientList[start].user)==0){
				if(enqueue(&clientList[start].queue, toClient)){
					strcpy(toClient, "messageQueued");
					sendMessage(fdlist[i], toClient);
				}else{
					strcpy(toClient, "messageNotQueued");
					sendMessage(fdlist[i], toClient);
				}
				memset(clientList[i].touser, 0, sizeof(clientList[i].touser));
				memset(clientList[i].buf, 0, sizeof(clientList[i].buf));
				break;
			}
		    }
                }
            }
        }
    }
}
