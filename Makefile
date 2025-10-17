CC := g++
CFLAGS := -Wall -Wextra -std=c++17 -Wno-narrowing

SOURCES = $(shell find . -type f -name "*.cpp")
OBJECTS := $(patsubst %.cpp,%.o,$(OBJECTS))
TARGET := bin/main

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(SOUR) $(TARGET)