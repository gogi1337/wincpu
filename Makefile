# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lole32 -loleaut32 -lwbemuuid
EXECUTABLE = wincpu.exe

# Directory settings
SRCDIR = src
BUILDDIR = build

# Object files (with build directory prefix)
OBJS = $(BUILDDIR)/main.o $(BUILDDIR)/hwid.o

# Default target
all: $(BUILDDIR) $(BUILDDIR)/$(EXECUTABLE)

# Create build directory
$(BUILDDIR):
	mkdir $(BUILDDIR)

# Link object files into executable
$(BUILDDIR)/$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) -o $(BUILDDIR)/$(EXECUTABLE) $(LDFLAGS)

# Compile main.c
$(BUILDDIR)/main.o: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile hwid.c
$(BUILDDIR)/hwid.o: $(SRCDIR)/hwid.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove build directory
clean:
	rmdir /s /q $(BUILDDIR)

# Run the program
run: $(BUILDDIR)/$(EXECUTABLE)
	.\$(BUILDDIR)\$(EXECUTABLE)

# Run the program in cycle mode
run-cycle: $(BUILDDIR)/$(EXECUTABLE)
	./$(BUILDDIR)/$(EXECUTABLE) -c

# Help target
help:
	@echo Available targets:
	@echo   all        - Build the program
	@echo   clean      - Remove build files
	@echo   run        - Run the program once
	@echo   run-cycle  - Run the program in cycle mode
	@echo   help       - Show this help message

.PHONY: all clean run run-cycle help 