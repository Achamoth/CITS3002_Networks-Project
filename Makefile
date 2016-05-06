# CITS3002 Project 2016
# Name:					Pradyumn Vij
# Student number(s):	21469477
# Date:					6/05/2016

#	Make file for Oldtrusty Client program
PROJECT = oldTrusty
HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)
OBJECTS = $(wildcard *.o)
EXECS = $(patsubst %.c,%,$(SOURCES))

CC = cc
CFLAGS = -std=gnu99 -Wall -pedantic -Werror

$(PROJECT) : $(OBJECTS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(PROJECT) $(OBJECTS)

$(OBJECTS): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(SOURCES)

clean:
	rm -f $(PROJECT) $(OBJECTS)
