#!/bin/bash

# Advanced Web Server Benchmark Script
# Demonstrates real-world performance testing

echo "🚀 Advanced Multithreaded Web Server Benchmark"
echo "=============================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if server is running
check_server() {
    echo -e "${BLUE}Checking if server is running...${NC}"
    if curl -s http://localhost:8080 > /dev/null; then
        echo -e "${GREEN}✅ Server is running${NC}"
        return 0
    else
        echo -e "${RED}❌ Server is not running${NC}"
        echo "Please start the server first: make run"
        return 1
    fi
}

# Test basic functionality
test_basic_functionality() {
    echo -e "\n${BLUE}Testing Basic Functionality${NC}"
    echo "------------------------"
    
    # Test main page
    echo -n "Testing main page... "
    if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/ | grep -q "200"; then
        echo -e "${GREEN}✅ OK${NC}"
    else
        echo -e "${RED}❌ FAILED${NC}"
    fi
    
    # Test CSS file
    echo -n "Testing CSS file... "
    if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/style.css | grep -q "200"; then
        echo -e "${GREEN}✅ OK${NC}"
    else
        echo -e "${RED}❌ FAILED${NC}"
    fi
    
    # Test JavaScript file
    echo -n "Testing JavaScript file... "
    if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/script.js | grep -q "200"; then
        echo -e "${GREEN}✅ OK${NC}"
    else
        echo -e "${RED}❌ FAILED${NC}"
    fi
    
    # Test API endpoint
    echo -n "Testing API endpoint... "
    if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/api/data.json | grep -q "200"; then
        echo -e "${GREEN}✅ OK${NC}"
    else
        echo -e "${RED}❌ FAILED${NC}"
    fi
    
    # Test metrics endpoint
    echo -n "Testing metrics endpoint... "
    if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/metrics | grep -q "200"; then
        echo -e "${GREEN}✅ OK${NC}"
    else
        echo -e "${RED}❌ FAILED${NC}"
    fi
    
    # Test 404 handling
    echo -n "Testing 404 handling... "
    if curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/nonexistent | grep -q "404"; then
        echo -e "${GREEN}✅ OK${NC}"
    else
        echo -e "${RED}❌ FAILED${NC}"
    fi
}

# Test caching
test_caching() {
    echo -e "\n${BLUE}Testing Caching Functionality${NC}"
    echo "----------------------------"
    
    # Test cache miss (first request)
    echo -n "First request (cache miss)... "
    start_time=$(date +%s%N)
    curl -s http://localhost:8080/style.css > /dev/null
    end_time=$(date +%s%N)
    first_time=$(( (end_time - start_time) / 1000000 ))
    echo -e "${YELLOW}${first_time}ms${NC}"
    
    # Test cache hit (second request)
    echo -n "Second request (cache hit)... "
    start_time=$(date +%s%N)
    curl -s http://localhost:8080/style.css > /dev/null
    end_time=$(date +%s%N)
    second_time=$(( (end_time - start_time) / 1000000 ))
    echo -e "${YELLOW}${second_time}ms${NC}"
    
    # Calculate speedup
    if [ $first_time -gt 0 ]; then
        speedup=$(( (first_time - second_time) * 100 / first_time ))
        echo -e "Cache speedup: ${GREEN}${speedup}%${NC}"
    fi
}

# Test concurrent requests
test_concurrent() {
    echo -e "\n${BLUE}Testing Concurrent Request Handling${NC}"
    echo "-----------------------------------"
    
    # Test with different numbers of concurrent requests
    for concurrent in 5 10 20; do
        echo -n "Testing $concurrent concurrent requests... "
        
        start_time=$(date +%s%N)
        
        # Start concurrent requests
        for i in $(seq 1 $concurrent); do
            curl -s http://localhost:8080/ > /dev/null &
        done
        
        # Wait for all to complete
        wait
        
        end_time=$(date +%s%N)
        duration=$(( (end_time - start_time) / 1000000 ))
        throughput=$(( concurrent * 1000 / duration ))
        
        echo -e "${GREEN}${duration}ms (${throughput} req/s)${NC}"
    done
}

# Test with Apache Bench if available
test_apache_bench() {
    if command -v ab &> /dev/null; then
        echo -e "\n${BLUE}Apache Bench Load Test${NC}"
        echo "---------------------"
        
        echo "Running Apache Bench test (100 requests, 10 concurrent)..."
        ab -n 100 -c 10 http://localhost:8080/ | grep -E "(Requests per second|Time per request|Failed requests)"
    else
        echo -e "\n${YELLOW}Apache Bench not available, skipping advanced load test${NC}"
    fi
}

# Test with curl for detailed timing
test_curl_timing() {
    echo -e "\n${BLUE}Detailed Response Time Analysis${NC}"
    echo "--------------------------------"
    
    echo "Testing response times for different endpoints:"
    
    endpoints=("/" "/style.css" "/script.js" "/about.html" "/api/data.json")
    
    for endpoint in "${endpoints[@]}"; do
        echo -n "  $endpoint: "
        time_info=$(curl -s -o /dev/null -w "%{time_total}" http://localhost:8080$endpoint)
        echo -e "${GREEN}${time_info}s${NC}"
    done
}

# Show live metrics
show_metrics() {
    echo -e "\n${BLUE}Live Server Metrics${NC}"
    echo "-------------------"
    
    echo "Fetching current metrics..."
    curl -s http://localhost:8080/metrics | grep -E "(Total Requests|Cache Hits|Cache Misses|Cache Hit Rate|Average Response Time|Cache Size)" | head -6
}

# Main benchmark function
run_benchmark() {
    if ! check_server; then
        exit 1
    fi
    
    test_basic_functionality
    test_caching
    test_concurrent
    test_curl_timing
    test_apache_bench
    show_metrics
    
    echo -e "\n${GREEN}✅ Benchmark completed!${NC}"
    echo -e "\n${BLUE}To run the interactive demo, visit: http://localhost:8080${NC}"
    echo -e "${BLUE}To view live metrics, visit: http://localhost:8080/metrics${NC}"
    echo -e "\n${YELLOW}For advanced load testing, run: python3 load_test.py${NC}"
}

# Run the benchmark
run_benchmark
