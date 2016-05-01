/*
	CITS3002 Project 2016
	Name:			Pradyumn Vij
	Student number:	21469477
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
