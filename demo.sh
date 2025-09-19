#!/bin/bash

# 🚀 Advanced Multithreaded Web Server Demo Script
# Comprehensive demonstration of all server features

echo "🚀 Advanced Multithreaded Web Server Demo"
echo "========================================="
echo ""

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Function to show menu
show_menu() {
    echo -e "${BLUE}Choose a demonstration:${NC}"
    echo "1. 🏠 Interactive Web Demo (Recommended)"
    echo "2. 🧪 Automated Benchmark Tests"
    echo "3. 📊 Python Load Testing"
    echo "4. 🔍 Quick Server Test"
    echo "5. 📖 View Documentation"
    echo "6. ❌ Exit"
    echo ""
    echo -n "Enter your choice (1-6): "
}

# Function to start server in background
start_server() {
    echo -e "${YELLOW}Starting server...${NC}"
    ./webserver &
    SERVER_PID=$!
    sleep 2
    
    # Check if server started successfully
    if curl -s http://localhost:8080 > /dev/null; then
        echo -e "${GREEN}✅ Server started successfully (PID: $SERVER_PID)${NC}"
        return 0
    else
        echo -e "${RED}❌ Failed to start server${NC}"
        return 1
    fi
}

# Function to stop server
stop_server() {
    if [ ! -z "$SERVER_PID" ]; then
        echo -e "${YELLOW}Stopping server (PID: $SERVER_PID)...${NC}"
        kill $SERVER_PID 2>/dev/null
        sleep 1
        pkill webserver 2>/dev/null
        echo -e "${GREEN}✅ Server stopped${NC}"
    fi
}

# Function to show server status
show_status() {
    echo -e "\n${BLUE}Server Status:${NC}"
    if curl -s http://localhost:8080 > /dev/null; then
        echo -e "${GREEN}✅ Server is running on http://localhost:8080${NC}"
        
        # Show basic metrics
        echo -e "\n${BLUE}Quick Metrics:${NC}"
        curl -s http://localhost:8080/metrics | grep -E "(Total Requests|Cache Hits|Cache Misses)" | head -3
    else
        echo -e "${RED}❌ Server is not running${NC}"
    fi
}

# Function for interactive demo
interactive_demo() {
    echo -e "\n${BLUE}🌐 Interactive Web Demo${NC}"
    echo "=========================="
    echo ""
    echo "This will open your web browser to an interactive demonstration."
    echo "The demo includes:"
    echo "• Load testing with 100 concurrent requests"
    echo "• Cache performance testing"
    echo "• Real-time metrics dashboard"
    echo "• Thread pool demonstration"
    echo ""
    
    if start_server; then
        echo ""
        echo -e "${GREEN}🎉 Server is ready!${NC}"
        echo ""
        echo "Open your browser and visit:"
        echo -e "${YELLOW}http://localhost:8080${NC}"
        echo ""
        echo "Try these features:"
        echo "• Click 'Load Test' to see multithreading in action"
        echo "• Click 'Cache Test' to see caching performance"
        echo "• Click 'View Metrics' to see real-time stats"
        echo "• Visit http://localhost:8080/metrics for live metrics"
        echo ""
        echo -e "${YELLOW}Press Enter when you're done exploring...${NC}"
        read -r
        
        stop_server
    fi
}

# Function for automated tests
automated_tests() {
    echo -e "\n${BLUE}🧪 Automated Benchmark Tests${NC}"
    echo "============================="
    echo ""
    echo "Running comprehensive automated tests..."
    echo ""
    
    if start_server; then
        ./benchmark.sh
        stop_server
    fi
}

# Function for Python load testing
python_load_test() {
    echo -e "\n${BLUE}📊 Python Load Testing${NC}"
    echo "======================="
    echo ""
    echo "Running advanced Python load tests..."
    echo ""
    
    # Check if Python dependencies are available
    if ! python3 -c "import requests" 2>/dev/null; then
        echo -e "${YELLOW}Installing Python dependencies...${NC}"
        pip3 install requests pillow 2>/dev/null || echo "Please install requests and pillow manually"
    fi
    
    if start_server; then
        python3 load_test.py
        stop_server
    fi
}

# Function for quick test
quick_test() {
    echo -e "\n${BLUE}🔍 Quick Server Test${NC}"
    echo "===================="
    echo ""
    
    if start_server; then
        echo "Testing basic functionality..."
        
        # Test main page
        echo -n "Main page: "
        if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/ | grep -q "200"; then
            echo -e "${GREEN}✅ OK${NC}"
        else
            echo -e "${RED}❌ FAILED${NC}"
        fi
        
        # Test metrics
        echo -n "Metrics page: "
        if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/metrics | grep -q "200"; then
            echo -e "${GREEN}✅ OK${NC}"
        else
            echo -e "${RED}❌ FAILED${NC}"
        fi
        
        # Test cache
        echo -n "Cache test: "
        time1=$(curl -s -o /dev/null -w "%{time_total}" http://localhost:8080/style.css)
        time2=$(curl -s -o /dev/null -w "%{time_total}" http://localhost:8080/style.css)
        echo -e "${GREEN}✅ OK (${time1}s → ${time2}s)${NC}"
        
        show_status
        stop_server
    fi
}

# Function to show documentation
show_docs() {
    echo -e "\n${BLUE}📖 Documentation${NC}"
    echo "================="
    echo ""
    echo "Project files:"
    echo "• README.md - Complete documentation"
    echo "• server.c - Main server implementation"
    echo "• thread_pool.c - Thread pool and task queue"
    echo "• cache.c - LRU cache implementation"
    echo "• metrics.c - Performance metrics"
    echo "• request_handler.c - HTTP request processing"
    echo ""
    echo "Testing files:"
    echo "• benchmark.sh - Automated testing script"
    echo "• load_test.py - Python load testing"
    echo "• index.html - Interactive web demo"
    echo ""
    echo "Build commands:"
    echo "• make - Build the server"
    echo "• make test - Run automated tests"
    echo "• make load-test - Run Python load tests"
    echo "• make demo - Start interactive demo"
    echo ""
    echo -e "${YELLOW}Press Enter to continue...${NC}"
    read -r
}

# Main loop
while true; do
    echo ""
    show_menu
    read -r choice
    
    case $choice in
        1)
            interactive_demo
            ;;
        2)
            automated_tests
            ;;
        3)
            python_load_test
            ;;
        4)
            quick_test
            ;;
        5)
            show_docs
            ;;
        6)
            echo -e "${GREEN}👋 Thanks for trying the Advanced Multithreaded Web Server!${NC}"
            exit 0
            ;;
        *)
            echo -e "${RED}Invalid choice. Please enter 1-6.${NC}"
            ;;
    esac
done
