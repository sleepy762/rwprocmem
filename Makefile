CC = g++
LD = g++

CFLAGS = -Wall -Wextra -pedantic -Iinclude -O2

SRCS = $(wildcard src/*.cpp) $(wildcard src/cmds/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = rwprocmem

all: $(OBJS) $(TARGET)

# Links the object files into an executable
$(TARGET): $(OBJS)
	$(LD) $^ -o $@

# Compiles every source file into an object file
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

