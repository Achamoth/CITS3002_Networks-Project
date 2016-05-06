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

//  Allow usage of types without specifying "struct"
typedef struct addrinfo addrinfo;
typedef struct sockaddr_in6 sockaddr_in6;

#if defined(__linux__)
extern  char  *strdup(const char *str);
#endif

//  Package accessible functions
extern void newRequiredMember(char *);
extern void freeCircleMembers();

extern bool uploadFile(char *, bool);
extern bool downloadFile(char *);
extern void getAddress(char *, int);
extern void listFiles();
extern void vouchForFile(char *, char *);
