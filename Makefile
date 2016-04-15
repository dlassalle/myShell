CC=g++
DEBUG=
CFLAGS=-c $(DEBUG)
LDFLAGS= $(DEBUG)
SOURCES=myshell.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=myshell

all: clean $(SOURCES) $(EXECUTABLE)

debug: clean
debug: DEBUG +=-g
debug: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)










