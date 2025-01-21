# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lole32 -loleaut32 -lwbemuuid
EXECUTABLE = wincpu.exe

# Object files
OBJS = main.o hwid.o

# Default target
all: $(EXECUTABLE)

# Link object files into executable
$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) -o $(EXECUTABLE) $(LDFLAGS)

# Compile main.c
main.o: main.c
	$(CC) $(CFLAGS) -c main.c

# Compile hwid.c
hwid.o: hwid.c
	$(CC) $(CFLAGS) -c hwid.c

# Clean target to remove executable and object files
clean:
	del $(EXECUTABLE) *.o

# Run the program
run: $(EXECUTABLE)
	.\$(EXECUTABLE)

# Run the program in cycle mode
run-cycle: $(EXECUTABLE)
	./$(EXECUTABLE) -c

# Help target
help:
	@echo Available targets:
	@echo   all        - Build the program
	@echo   clean      - Remove build files
	@echo   run        - Run the program once
	@echo   run-cycle  - Run the program in cycle mode
	@echo   help       - Show this help message

.PHONY: all clean run run-cycle help 