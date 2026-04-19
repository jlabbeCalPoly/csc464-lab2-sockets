/******************************************************************************
* myServer.c
* 
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>

#include "networks.h"
#include "safeUtil.h"
#include "handlePDU.h"
#include "pollLib.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);

void addNewSocket(int mainServerSocket) {
	int clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG);
	addToPollSet(clientSocket);
}

void processClient(int clientSocket) {
	recvFromClient(clientSocket);
}

void serverControl(int portNumber) {
	int mainServerSocket = 0;   //socket descriptor for the server socket
	
	//create the server socket, add it to the poll set
	mainServerSocket = tcpServerSetup(portNumber);
	setupPollSet();
	addToPollSet(mainServerSocket);

	while (1) {
		int socket = pollCall(0);
		if (socket != -1) {
			if (socket == mainServerSocket) {
				addNewSocket(mainServerSocket);
			} else {
				processClient(socket);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	int portNumber = checkArgs(argc, argv);
	serverControl(portNumber);

	return 0;
}

void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	if (messageLen > 0)
	{
		printf("Socket %d: Message received, length: %d Data: %s\n", clientSocket, messageLen, dataBuffer);
		
		
		// send it back to client (just to test sending is working... e.g. debugging)
		messageLen = safeSend(clientSocket, dataBuffer, messageLen, 0);
		// printf("Socket %d: msg sent: %d bytes, text: %s\n", clientSocket, messageLen, dataBuffer);
		
	}
	else
	{
		printf("Socket %d: Connection closed by other side\n", clientSocket);

		// remove from the poll set and close the socket
		removeFromPollSet(clientSocket);
		close(clientSocket);
	}
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

