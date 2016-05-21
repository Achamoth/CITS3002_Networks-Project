PROJECT = oldTrusty
SOURCES = $(wildcard *.java)
CLASS = $(wildcard *.class)

JC = javac
LIBS = lib/guava-19.0.jar:lib/jgrapht-core-0.9.1.jar:lib/bcprov-jdk15on-154.jar:lib/bcpkix-jdk15on-154.jar:.
JFLAGS = -cp

$(PROJECT) : $(CLASS)
	$(JC) $(JFLAGS) $(LIBS) $(SOURCES)

clean:
	rm -f $(CLASS)
	