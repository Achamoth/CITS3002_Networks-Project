/*
	CITS3002 Project 2016
	Name:			Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
	Student number:	21469477
	Date:			d/m/2015
*/
#include "client.h"

/*
	openConnection
	Open a socket and connect using TCP protocol
	@param host 	Server address
	@param port 	Server port
	@return 		Open socket
*/
int openConnection(const char *host, const char *port){
	addrinfo hostReq;
	addrinfo *hostFound;
	int addrInfo_Error;
	int socketDescriptor;

	//  Make sure struct is initialised cleanly
	memset(&hostReq, 0, sizeof(hostReq));
	//  Set the host requirements addrinfo
	hostReq.ai_family = AF_UNSPEC;
    //  TCP stream (SSL requires) reliability
	hostReq.ai_socktype = SOCK_STREAM;
	hostReq.ai_protocol = IPPROTO_TCP;

	// Error check on getaddrinfo
	if((addrInfo_Error = getaddrinfo(host, port, &hostReq, &hostFound)) != 0){
		fprintf(stderr, "%s: Get Address Error: %s\n", programName, 
			gai_strerror(addrInfo_Error));
		usage();
	}

	// Create a socket descriptor
	socketDescriptor = socket(hostFound->ai_family,
								hostFound->ai_socktype,
									hostFound->ai_protocol);
	if(socketDescriptor < 0){
		perror("Socket Error");
		exit(EXIT_FAILURE);
	}

	// Connect to socket
	if(connect(socketDescriptor, hostFound->ai_addr, hostFound->ai_addrlen) < 0){
		close(socketDescriptor);
		perror("Connect Error");
		exit(EXIT_FAILURE);
	}
	// Free the addrinfo struct
	freeaddrinfo(hostFound);

	// Return socket descript to close after operation 
	return socketDescriptor;
}
