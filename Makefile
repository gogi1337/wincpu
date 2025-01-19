# Compiler settings
CC = gcc
CFLAGS = -Wall -O2

# Target executable name
TARGET = wincpu.exe

# Source files
SOURCES = main.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Libraries
LIBS = -lpowrprof

# Default target
all: $(TARGET)

# Link the program
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	del /F /Q $(TARGET) *.o

# Run the program
run: $(TARGET)
	./$(TARGET)

# Run the program in cycle mode
run-cycle: $(TARGET)
	./$(TARGET) -c

# Help target
help:
	@echo Available targets:
	@echo   all        - Build the program
	@echo   clean      - Remove build files
	@echo   run        - Run the program once
	@echo   run-cycle  - Run the program in cycle mode
	@echo   help       - Show this help message

.PHONY: all clean run run-cycle help 