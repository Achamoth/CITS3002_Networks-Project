/*
 CITS3002 Project 2016
 Name:			Pradyumn Vij, Ammar Abu Shamleh
 Student number:	21469477, 21521274
 Date:			d/m/2015
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


//------------------------------------------------------------------
// Enum action struct to determine purpose of client message
//------------------------------------------------------------------
typedef enum {
	NONE = 0,
	PUSH = 1,
	PULL = 2,
	LIST = 3,
	CHECK = 4,
	VOUCH = 5,
	PASS = 6,
	FAIL = 7,
} actionType;

//------------------------------------------------------------------
// POSIX defined container struct from netdb.h
// As described in 
// http://beej.us/guide/bgnet/output/html/multipage/ipstructsdata.html
//------------------------------------------------------------------
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
extern int openConnection(const char *, const char *);
extern void usage();
extern void sendMessage(char *, char *, actionType, char *, char*, 
							int, char*);
extern void newRequiredMember(char *);
extern void freeCircleMembers();
extern bool uploadFile(char *, bool);
extern bool downloadFile(char *);
extern void getAddress(char *, int);
extern void listFiles();
extern void vouchForFile(char *, char *);

//------------------------------------------------------------------
// Package accessible variables
//------------------------------------------------------------------
extern const char* programName;