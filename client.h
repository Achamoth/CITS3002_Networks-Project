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
extern  int fileno(const FILE *fp);
#endif

void newRequiredMember(char *newMember);
void freeCircleMembers();

bool uploadFile(char *fileName, bool isCert);
bool downloadFile(char *fileName);
void getAddress(char *hostName, int port);
void listFiles();
void vouchForFile(char *filename, char *certname);
