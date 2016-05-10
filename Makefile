# CITS3002 Project 2016
# Name:                 Pradyumn Vij
# Student number(s):    21469477
# Date:                 6/05/2016

#   Make file for Oldtrusty Client program
PROJECT = oldTrusty
HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)
OBJECTS = $(wildcard *.o)

CC = cc
CFLAGS = -std=c99 -Wall -pedantic -Werror
LIBS = -lssl -lcrypto

$(PROJECT) : $(OBJECTS)
    $(CC) $(CFLAGS) $(LIBS) $(SOURCES) -o $(PROJECT) $(OBJECTS) $(LIBS)

$(OBJECTS): $(SOURCES) $(HEADERS)
    $(CC) $(CFLAGS) $(LIBS) -c $(SOURCES)

clean:
    rm -f $(PROJECT) $(OBJECTS)
