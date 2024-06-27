CXXFLAGS=-std=c++11 -O2
LDLIBS=-lm
SOURCES=./src/node.cpp ./src/encryption.cpp ./src/main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=logic_lock
INCLUDES=./src/*.h

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDLIBS) -o $@

%.o: %.cpp ${INCLUDES}

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
