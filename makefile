CC = g++
CFLAGS= -g -c -Wall `sdl-config --cflags`
LDFLAGS= `sdl-config --libs`
SOURCES=ass2.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=histEqImage

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
	rm $(EXECUTABLE)

