# Makefile for Multithreaded Web Server

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -g
LDFLAGS = -pthread

# Source files
SOURCES = server.c thread_pool.c metrics.c request_handler.c cache.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = webserver

# Load balancer
LB_SOURCES = load_balancer.c
LB_OBJECTS = $(LB_SOURCES:.c=.o)
LB_TARGET = load_balancer

# Default target
all: $(TARGET) $(LB_TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Build the load balancer
$(LB_TARGET): $(LB_OBJECTS)
	$(CC) $(LB_OBJECTS) -o $(LB_TARGET) $(LDFLAGS)

# Compile source files to object files
%.o: %.c server.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compile load balancer object files (no server.h dependency)
load_balancer.o: load_balancer.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET) $(LB_OBJECTS) $(LB_TARGET)

# =============================================================================
# ESSENTIAL COMMANDS
# =============================================================================

# Start complete load balancer setup (RECOMMENDED)
start-lb: $(TARGET) $(LB_TARGET)
	@echo "🚀 Starting complete load balancer setup..."
	@./start_lb.sh

# Run single webserver
run: $(TARGET)
	@echo "🌐 Starting webserver on port 8080..."
	@echo "Visit http://localhost:8080"
	@echo "Press Ctrl+C to stop"
	@./$(TARGET)

# Stop all services
stop:
	@echo "🛑 Stopping all services..."
	@pkill -f "webserver" || true
	@pkill -f "load_balancer" || true
	@echo "All services stopped."

# =============================================================================
# TESTING COMMANDS
# =============================================================================

# Run automated benchmark tests
test: $(TARGET)
	@echo "🧪 Running benchmark tests..."
	@./$(TARGET) &
	@sleep 2
	@./benchmark.sh
	@pkill webserver || true

# Run Python load testing
load-test: $(TARGET)
	@echo "📊 Running load tests..."
	@./$(TARGET) &
	@sleep 2
	@python3 load_test.py
	@pkill webserver || true

# =============================================================================
# BUILD COMMANDS
# =============================================================================

# Debug build
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET) $(LB_TARGET)

# Release build
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET) $(LB_TARGET)

# =============================================================================
# HELP
# =============================================================================

help:
	@echo "🎯 ESSENTIAL COMMANDS:"
	@echo "  make start-lb    - Start complete load balancer setup (RECOMMENDED)"
	@echo "  make run         - Run single webserver on port 8080"
	@echo "  make stop        - Stop all running services"
	@echo ""
	@echo "🧪 TESTING:"
	@echo "  make test        - Run benchmark tests"
	@echo "  make load-test   - Run Python load tests"
	@echo ""
	@echo "🔧 BUILD:"
	@echo "  make all         - Build webserver and load balancer"
	@echo "  make clean       - Remove build artifacts"
	@echo "  make debug       - Build with debug flags"
	@echo "  make release     - Build with optimization"
	@echo ""
	@echo "❓ HELP:"
	@echo "  make help        - Show this help message"

.PHONY: all clean run start-lb stop test load-test debug release help