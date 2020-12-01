/*
Authentication Client and Server
Server sends the following messages to the client:

- PROCEED if user enters the correct username and password
- DENIED if user enters the wrong username and password continuously for 3 times
- "You have x tries left." letting the user know the amount of tries left 

Student Number: D19126659
Student Name: Aaron Fu Siang Ann
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include "Practical.h" 
#include <unistd.h> 
#include <sys/stat.h> 

#define PROCEED "PROCEED" // Message to be sent to client
#define DENIED "DENIED" // Message to be sent to client

static const int MAXPENDING = 1; 

int main(int argc, char *argv[]) 
{
	int numBytes = 0, count = 3; 
	char recvbuffer[BUFSIZE], // Buffer for receiving data from client
	sendbuffer[BUFSIZE], // Buffer for sending data to client
	username[20]={""}, // Array to store username sent from client
	password[20]={""}, // Array to store password sent from client 
	usernameServer[20] = {"admin"}, // Array to store server username
	passwordServer[20] = {"pass"}; // Array to store server password
 	
	// Test for correct number of arguments
	if (argc != 2)
    		DieWithUserMessage("Parameter(s)", "<Server Port>");
  	
	// First argument: local port
	in_port_t servPort = atoi(argv[1]);
	
	// Creates the socket for incoming connections
	// Socket descriptor for server
	int servSock;
	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    		DieWithSystemMessage("socket() failed");
  	
	// Construct local address structure
	struct sockaddr_in servAddr;				// Local address
  	memset(&servAddr, 0, sizeof(servAddr));			// Zero out structure
  	servAddr.sin_family = AF_INET;				// IPv4 address family
  	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);		// Any incoming interface
  	servAddr.sin_port = htons(servPort);			// Local port

	// Bind to the local address
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    		DieWithSystemMessage("bind() failed");

	// Mark the socket so it will listen for incoming connections 
	if (listen(servSock, MAXPENDING) < 0)
   	 	DieWithSystemMessage("listen() failed");

	// Infinite for loop, runs forever
  	for (;;)
	{
		// Wait for a client to connect
		int clntSock = accept(servSock, (struct sockaddr *) NULL, NULL);

    		if (clntSock < 0)
      			DieWithSystemMessage("accept() failed");

		// Keeps track of the number of attemps, while not equals to zero keep running
    		while(count != 0)
    		{
    			while ((numBytes = recv(clntSock, recvbuffer, (BUFSIZE-1), 0)) > 0)
			{
				recvbuffer[numBytes] = '\0';
				fputs(recvbuffer, stdout);

				if(strstr(recvbuffer, "\r\n\r\n") > 0)
					break;
    			}

			// Display error if failed to receive data from client
    			if (numBytes < 0)
      				DieWithSystemMessage("recv() failed");

			// Reads the receiving buffer
			// Stores login credentials into server's username and password variable
			sscanf(recvbuffer, "%s %s", username, password);

			/* Validates username and password,
			   compares server's username & password with
			   client's username & password
			   Do this if username and password is correct */
			if((strcmp(usernameServer, username) == 0) && (strcmp(passwordServer, password) == 0))
			{
				// Stores PROCEED message into sendbuffer
				snprintf(sendbuffer, sizeof(sendbuffer),"%s", PROCEED);

				// Sends the PROCEED message stored in sendbuffer to the client
				ssize_t numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0);

				// Display error message if message failed to send
				if (numBytesSent < 0)
      					DieWithSystemMessage("send() failed");

				// Close the client socket 
				close(clntSock);
			}
			// Do this if username or password is invalid
			else
			{
				// Reduce count / tries everytime user enters invalid credentials
				count--;
				// When count reaches zero do this
				if(count == 0)
				{
					// Stores DENIED message into sendbuffer
					snprintf(sendbuffer, sizeof(sendbuffer), "%s", DENIED);

					// Sends the DENIED message stored in sendbuffer to the client
					ssize_t numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0);

					// Display error message if message failed to send
					if (numBytesSent < 0)
      						DieWithSystemMessage("send() failed");

					// Close the client socket 
					close(clntSock);
				}
				// When count is not equals zero
				else
				{
					// Stores number of tries message into sendbuffer
					snprintf(sendbuffer, sizeof(sendbuffer), "You have %d tries left.\r\n", count);

					// Sends the number of tries message stored in sendbuffer to the client
					ssize_t numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0);

					// Display error message if message failed to send
					if (numBytesSent < 0)
      						DieWithSystemMessage("send() failed");

				}
			}
		}
    	}
}
