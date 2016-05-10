/*
	CITS3002 Project 2016
	Name:			Ammar Abu Shamleh, Pradyumn Vij, Goce Krlevski 
	Student number:	21469477
	Date:			d/m/2015
*/
#include "client.h"

/*
	openTCPConnection
	Open a socket and connect using TCP protocol
	@param host 	Server address
	@param port 	Server port
	@return 		Open socket
*/
int openTCPConnection(const char *host, const char *port){
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


/*
	secureConnection
	Intialise SSL context and negotiate secure transmission with
	server
*/
void secureConnection(const char* host, const char* port){
	//  Initialise SSL library
	SSL_load_error_strings();
	SSL_library_init();	//  Loacs encryption and hashing for SSL program
	OpenSSL_add_all_algorithms(); // cryptogrophy
	//  Create SSL context - client in this case using SSLv3
	SSL_CTX *sslContext = SSL_CTX_new(SSLv3_client_method());
	if(sslContext == NULL){
		fprintf(stderr, "%s: SSL: Failed to create new context.\n", programName);
		exit(EXIT_FAILURE);
	}

	//  Public Key
	//  Load the client certificate (need to have position in project folder for certs)
	if(SSL_CTX_use_certificate_file(sslContext, RSA_CLIENT_CERT, 
													SSL_FILETYPE_PEM) < 1){
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	//  Private Key
	//  Load private key corresponding to client certificate
	if(SSL_CTX_check_PrivateKey_file(sslContext, RSA_CLIENT_KEY, 
													SSL_FILETYPE_PEM) < 1){
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	//  Check if certificate and key provided match
	if(!SSL_CTX_check_private_key(sslContext)){
		fprintf(stderr, "%s: SSL: Private key does not match certificate public key.\n",
					programName);
		exit(EXIT_FAILURE);
	}

	//  Load RSA certificate authority into SSL context, allow client to verify server
	//  certificate. *****remember to set rsa certificate authority crt file
	if(!SSL_CTX_load_verify_locations(sslContext, RSA_CLIENT_CA_CERT, NULL)){
		ERR_print_errors_fp(stderr);
		exit(EXIT_FAILURE);
	}

	//  Set flag in context to require server certificate verification
	SSL_CTX_set_verify(sslContext, SSL_VERIFY_PEER, NULL);
	SSL_CTX_set_verify_depth(sslContext,1);

	//  Connect to server via TCP
	int socketDescriptor = openTCPConnection(host, port);

	// Create SSL structure
	SSL *ssl = SSL_new(sslContext);
	if(ssl == NULL){
		fprintf(stderr, "%s: SSL: Failed to create new context.\n", programName);
		exit(EXIT_FAILURE);
	}

	//TODO perform SSL handshake, get server's certificate start communication




	//SSL_shutdown();
	//SSL_free
	SSL_CTX_free(sslContext);
}