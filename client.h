/*
 CITS3002 Project 2016
 Name:			Pradyumn Vij, Ammar Abu Shamleh
 Student number:	21469477, 21521274
 Date:			d/m/2015
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#if defined(__linux__)
extern  char  *strdup(const char *str);
#endif

//  Package accessible functions
extern void newRequiredMember(char *);
extern void freeCircleMembers();

extern bool uploadFile(char *, bool);
extern bool downloadFile(char *);
extern getAddress(char *, int);
extern void listFiles();
extern void vouchForFile(char *, char *);
