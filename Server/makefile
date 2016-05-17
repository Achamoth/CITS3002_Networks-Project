PROJECT = oldTrusty
SOURCES = $(wildcard *.java)
CLASS = $(wildcard *.class)

JC = javac
LIBS = lib/jgrapht-core-0.9.1.jar:.
JFLAGS = -cp

$(PROJECT) : $(CLASS)
	$(JC) $(JFLAGS) $(LIBS) $(SOURCES)

clean:
	rm -f $(CLASS)
	