CC = g++
CFLAGS = -Wall -Wextra -pedantic -Iinclude -O2

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = rwprocmem

all: $(OBJS) $(TARGET)

# Links the object files into an executable
$(TARGET): $(OBJS)
	$(CC) $^ -o $@

# Compiles every source file into an object file
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f src/*.o $(TARGET)

