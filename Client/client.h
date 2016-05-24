/*
	 CITS3002 Project 2016
	 Name:			Pradyumn Vij, Ammar Abu Shamleh
	 Student number: 21521274, 21469477
	 Date:           May 2016
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>			// interpret error codes
#include <unistd.h>
#include <string.h>			// string manipulation
#include <getopt.h>			// getopt
#include <stdbool.h>		// explicit bools
#include <sys/types.h>		// portability for socket() call
//  Network includes
#include <sys/socket.h>		// socket funcitons
#include <netdb.h>			// getaddrinfo()
#include <netinet/in.h>		// sockaddr_in def
//  OpenSSL includes
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>

//------------------------------------------------------------------
// Preprocessor defined constants for control communication with server
//-----------------------------------------------------------------
#define FILE_TRUSTWORTHY 8
#define FILE_UNTRUSTWORTHY 9
#define ACKNOWLEDGMENT 10
#define FILE_NOT_FOUND 15
#define FILE_FOUND 16
#define MEMBER_REQUIRED 17
#define MEMBER_NOT_REQUIRED 18
#define CLIENT_VALID_FILE 19
#define CLIENT_INVALID_FILE 20
/*#define PUSH 1
#define PULL 2
#define PUSH_CERT 3
#define LIST 4
#define VOUCH 5
#define FILE_TRUSTWORTHY 8
#define FILE_UNTRUSTWORTHY 9
#define ACKNOWLEDGMENT 10
#define FILE_NOT_FOUND 15
#define FILE_FOUND 16*/

//------------------------------------------------------------------
// Enum action struct to determine purpose of client message
//------------------------------------------------------------------
typedef enum {
	NONE = 0,
	PUSH = 1,
	PULL = 2,
    PUSH_CERT = 3,
	LIST = 4,
	CHECK = 5,
	VOUCH = 6,
	PASS = 7,
	FAIL = 8,
} actionType;

//------------------------------------------------------------------
// POSIX defined container struct from netdb.h
// As described in 
// http://beej.us/guide/bgnet/output/html/multipage/ipstructsdata.html
//------------------------------------------------------------------
#if defined(__linux__)
typedef struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
} addrinfo;
#endif

//------------------------------------------------------------------
// POSIX defined functions
//------------------------------------------------------------------
#if defined(__linux__)
	extern char  *strdup(const char *str);
	extern int getaddrinfo(const char *node, const char *service,
							const struct addrinfo *hints,
								struct addrinfo **res);
	extern const char *gai_strerror(int errcode);
	extern void freeaddrinfo(struct addrinfo *res);
#endif

//------------------------------------------------------------------
// Package accessible functions
//------------------------------------------------------------------
extern void closeConnection();
extern void usage();

extern void parseRequest(char *, char *, actionType, char *, char*,
							int, char*, bool);

extern SSL *secureConnection(const char*, const char *);
//extern void sendFile(SSL *, char* , char*);
//extern int readResponse(SSL *);

//extern void newRequiredMember(char *);
//extern void freeCircleMembers();
//extern void downloadFile(SSL *, char *);
//extern void getAddress(char *, int);
//extern void listFiles();
//extern void vouchForFile(char *, char *);

//------------------------------------------------------------------
// Package accessible variables
//------------------------------------------------------------------
extern const char* programName;
