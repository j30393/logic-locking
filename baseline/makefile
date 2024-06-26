CC=g++
LDFLAGS=-std=c++11 -lm -O2
SOURCES=./src/node.cpp ./src/encryption.cpp ./src/main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=logic_lock
INCLUDES=./src/inc/*

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o: %.cpp ${INCLUDES}
	$(CC) $(LDFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
