CC=g++
LDFLAGS=-std=c++11 -lm -g
SOURCES=./src/node.cpp ./src/cone.cpp ./src/encryption.cpp ./src/main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=logic_lock
INCLUDES=./src/inc/*

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp ${INCLUDES}
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
