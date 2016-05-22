# CITS3002 Project 2016
# Name:                 Ammar Abu Shamleh, Pradyumn Vij
# Student number(s):    21521274, 21469477

#   Make file for oldTrusty Server program

PROJECT = oldTrusty
SOURCES = $(wildcard *.java)
CLASS = $(wildcard *.class)

JC = javac
LIBS = lib/jgrapht-core-0.9.1.jar:lib/bcprov-jdk15on-154.jar:lib/bcpkix-jdk15on-154.jar:lib/gs-core-1.3.jar:.
JFLAGS = -cp

$(PROJECT) : $(CLASS)
	$(JC) $(JFLAGS) $(LIBS) $(SOURCES)

clean:
	rm -f $(CLASS)
	