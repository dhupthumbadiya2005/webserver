# Makefile for Multithreaded Web Server

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -g
LDFLAGS = -pthread

# Source files
SOURCES = server.c thread_pool.c metrics.c request_handler.c cache.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = webserver

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.c server.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install dependencies (if needed)
install-deps:
	@echo "No external dependencies required"

# Run the server
run: $(TARGET)
	./$(TARGET)

# Test the server
test: $(TARGET)
	@echo "Starting server in background..."
	@./$(TARGET) &
	@sleep 2
	@echo "Running tests..."
	@./benchmark.sh
	@pkill webserver || true

# Load test with Python
load-test: $(TARGET)
	@echo "Starting server in background..."
	@./$(TARGET) &
	@sleep 2
	@echo "Running load tests..."
	@python3 load_test.py
	@pkill webserver || true

# Demo - start server and open browser
demo: $(TARGET)
	@echo "Starting server..."
	@echo "Visit http://localhost:8080 for interactive demo"
	@echo "Press Ctrl+C to stop"
	@./$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)

# Install Python dependencies for testing
install-test-deps:
	@echo "Installing Python dependencies..."
	@pip3 install requests pillow || echo "Please install requests and pillow manually"

# Help
help:
	@echo "Available targets:"
	@echo "  all           - Build the webserver (default)"
	@echo "  clean         - Remove build artifacts"
	@echo "  run           - Build and run the webserver"
	@echo "  test          - Run automated benchmark tests"
	@echo "  load-test     - Run Python load testing"
	@echo "  demo          - Start server and show demo info"
	@echo "  debug         - Build with debug flags"
	@echo "  release       - Build with optimization flags"
	@echo "  install-test-deps - Install Python testing dependencies"
	@echo "  help          - Show this help message"

.PHONY: all clean install-deps run test load-test demo debug release install-test-deps help
