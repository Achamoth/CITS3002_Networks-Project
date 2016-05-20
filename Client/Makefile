# CITS3002 Project 2016
# Name:                 Pradyumn Vij
# Student number(s):    21469477
# Date:                 6/05/2016

#   Make file for Oldtrusty Client program
EXEC = oldTrustyClient
HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)
OBJECTS = $(wildcard *.o)

CC = cc
CFLAGS = -std=c99 -Wall -pedantic -Werror -Wno-deprecated
LDLIBS = -lssl -lcrypto

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXEC) $(LDLIBS)

$(OBJECTS): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(SOURCES) $(LDLIBS)

clean:
	rm -f $(OBJECTS) $(EXEC)
