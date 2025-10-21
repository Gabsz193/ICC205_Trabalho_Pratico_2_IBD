CC := g++
CFLAGS := -Wall -Wextra -std=c++17 -Wno-narrowing

SOURCES = $(shell find . -type f -name "*.cpp")
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
TARGET := bin/main

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJECTS) $(TARGET)