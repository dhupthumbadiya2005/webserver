#!/bin/bash

# Script to start load balancer with backend servers
# This script ensures clean startup and proper cleanup

echo "=== Starting Load Balancer Setup ==="

# Kill any existing processes
echo "Cleaning up existing processes..."
pkill -f "webserver" 2>/dev/null || true
pkill -f "load_balancer" 2>/dev/null || true
sleep 3

# Start backend servers
echo "Starting backend servers on ports 8081-8084..."
echo "Starting backend 1 on port 8081..."
PORT=8081 ./webserver &
echo "Starting backend 2 on port 8082..."
PORT=8082 ./webserver &
echo "Starting backend 3 on port 8083..."
PORT=8083 ./webserver &
echo "Starting backend 4 on port 8084..."
PORT=8084 ./webserver &
BACKEND_PID=$!

# Wait for backends to start
echo "Waiting for backend servers to start..."
sleep 5

# Check if backends are running
BACKEND_COUNT=$(ps aux | grep -c "./webserver" | grep -v grep)
if [ $BACKEND_COUNT -lt 4 ]; then
    echo "Warning: Only $BACKEND_COUNT backend servers are running (expected 4)"
fi

# Check if port 8085 is free
if lsof -i :8085 >/dev/null 2>&1; then
    echo "❌ Port 8085 is still in use. Force killing processes..."
    pkill -9 -f "load_balancer" 2>/dev/null || true
    sleep 2
fi

# Start load balancer
echo "Starting load balancer on port 8085..."
./load_balancer &
LB_PID=$!

# Wait a moment for load balancer to start
sleep 3

# Test the setup
echo "Testing load balancer..."
if curl -s http://localhost:8085/ > /dev/null; then
    echo " Load balancer is working correctly!"
    echo " Access your load balanced server at: http://localhost:8085"
    echo " Backend servers running on ports: 8081, 8082, 8083, 8084"
    echo ""
    echo "Press Ctrl+C to stop all services"
    
    # Wait for user interrupt
    trap 'echo ""; echo "Stopping all services..."; pkill -f "webserver"; pkill -f "load_balancer"; echo "All services stopped."; exit 0' INT
    wait
else
    echo " Load balancer test failed"
    echo "Stopping all services..."
    pkill -f "webserver"
    pkill -f "load_balancer"
    exit 1
fi
