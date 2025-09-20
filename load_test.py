#!/usr/bin/env python3
"""
Advanced Load Testing Script for Multithreaded Web Server
Demonstrates concurrent request handling and performance metrics
"""

import requests
import threading
import time
import statistics
from concurrent.futures import ThreadPoolExecutor, as_completed
import json

class LoadTester:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.results = []
        self.lock = threading.Lock()
    
    def clear_results(self):
        """Clear all previous test results"""
        with self.lock:
            self.results = []
    
    def single_request(self, endpoint="/", request_id=0):
        """Make a single HTTP request and record timing"""
        start_time = time.time()
        try:
            response = requests.get(f"{self.base_url}{endpoint}", timeout=10)
            end_time = time.time()
            
            result = {
                'request_id': request_id,
                'endpoint': endpoint,
                'status_code': response.status_code,
                'response_time': (end_time - start_time) * 1000,  # Convert to ms
                'success': response.status_code == 200,
                'content_length': len(response.content)
            }
            
            with self.lock:
                self.results.append(result)
            
            return result
            
        except Exception as e:
            end_time = time.time()
            result = {
                'request_id': request_id,
                'endpoint': endpoint,
                'status_code': 0,
                'response_time': (end_time - start_time) * 1000,
                'success': False,
                'error': str(e)
            }
            
            with self.lock:
                self.results.append(result)
            
            return result
    
    def run_load_test(self, num_requests=100, num_threads=10, endpoints=None):
        """Run a comprehensive load test"""
        if endpoints is None:
            endpoints = ["/", "/style.css", "/script.js", "/about.html", "/api/data.json"]
        
        # Clear previous results
        self.results = []
        
        print(f" Starting Load Test")
        print(f"   Requests: {num_requests}")
        print(f"   Threads: {num_threads}")
        print(f"   Endpoints: {endpoints}")
        print(f"   Server: {self.base_url}")
        print("-" * 50)
        
        start_time = time.time()
        
        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            # Submit all requests
            futures = []
            for i in range(num_requests):
                endpoint = endpoints[i % len(endpoints)]
                future = executor.submit(self.single_request, endpoint, i)
                futures.append(future)
            
            # Wait for completion and show progress
            completed = 0
            for future in as_completed(futures):
                completed += 1
                if completed % 10 == 0 or completed == num_requests:
                    print(f"   Progress: {completed}/{num_requests} requests completed")
        
        end_time = time.time()
        total_duration = end_time - start_time
        
        self.print_results(total_duration, num_requests)
    
    def print_results(self, total_duration, num_requests):
        """Print comprehensive test results"""
        if not self.results:
            print("❌ No results to display")
            return
        
        successful_requests = [r for r in self.results if r['success']]
        failed_requests = [r for r in self.results if not r['success']]
        
        response_times = [r['response_time'] for r in successful_requests]
        
        print("\n" + "="*60)
        print(" LOAD TEST RESULTS")
        print("="*60)
        
        print(f"Total Requests: {num_requests}")
        print(f"Successful: {len(successful_requests)}")
        print(f"Failed: {len(failed_requests)}")
        print(f"Success Rate: {len(successful_requests)/num_requests*100:.1f}%")
        print(f"Total Duration: {total_duration:.2f}s")
        print(f"Requests/Second: {num_requests/total_duration:.1f}")
        
        if response_times:
            print(f"\nResponse Time Statistics:")
            print(f"  Average: {statistics.mean(response_times):.2f}ms")
            print(f"  Median: {statistics.median(response_times):.2f}ms")
            print(f"  Min: {min(response_times):.2f}ms")
            print(f"  Max: {max(response_times):.2f}ms")
            print(f"  95th Percentile: {sorted(response_times)[int(len(response_times)*0.95)]:.2f}ms")
        
        # Endpoint breakdown
        endpoint_stats = {}
        for result in self.results:
            endpoint = result['endpoint']
            if endpoint not in endpoint_stats:
                endpoint_stats[endpoint] = {'total': 0, 'success': 0, 'times': []}
            endpoint_stats[endpoint]['total'] += 1
            if result['success']:
                endpoint_stats[endpoint]['success'] += 1
                endpoint_stats[endpoint]['times'].append(result['response_time'])
        
        print(f"\nEndpoint Breakdown:")
        for endpoint, stats in endpoint_stats.items():
            success_rate = stats['success'] / stats['total'] * 100
            avg_time = statistics.mean(stats['times']) if stats['times'] else 0
            print(f"  {endpoint}: {stats['success']}/{stats['total']} ({success_rate:.1f}%) - {avg_time:.2f}ms avg")
        
        if failed_requests:
            print(f"\nFailed Requests:")
            for req in failed_requests[:5]:  # Show first 5 failures
                print(f"  Request {req['request_id']}: {req.get('error', 'Unknown error')}")
    
    def cache_test(self):
        """Test cache effectiveness"""
        print("\n🧪 CACHE EFFECTIVENESS TEST")
        print("-" * 40)
        
        # Clear previous results
        self.results = []
        
        # Test the same file multiple times
        test_file = "/style.css"
        times = []
        
        for i in range(10):
            result = self.single_request(test_file, i)
            if result['success']:
                times.append(result['response_time'])
                print(f"  Request {i+1}: {result['response_time']:.2f}ms")
        
        if len(times) >= 2:
            first_request = times[0]
            avg_cached = statistics.mean(times[1:])
            speedup = (first_request - avg_cached) / first_request * 100
            
            print(f"\nCache Performance:")
            print(f"  First request (cache miss): {first_request:.2f}ms")
            print(f"  Average cached requests: {avg_cached:.2f}ms")
            print(f"  Cache speedup: {speedup:.1f}%")
    
    def concurrent_test(self):
        """Test concurrent request handling"""
        print("\n🧵 CONCURRENT REQUEST TEST")
        print("-" * 40)
        
        # Clear previous results
        self.results = []
        
        endpoints = ["/", "/style.css", "/script.js", "/about.html", "/api/data.json"]
        
        # Test with different concurrency levels
        for concurrency in [5, 10, 20, 50]:
            print(f"\nTesting {concurrency} concurrent requests...")
            
            start_time = time.time()
            with ThreadPoolExecutor(max_workers=concurrency) as executor:
                futures = [executor.submit(self.single_request, endpoints[i % len(endpoints)], i) 
                          for i in range(concurrency)]
                
                results = [future.result() for future in as_completed(futures)]
            
            end_time = time.time()
            duration = end_time - start_time
            successful = sum(1 for r in results if r['success'])
            
            print(f"  Duration: {duration:.2f}s")
            print(f"  Successful: {successful}/{concurrency}")
            print(f"  Throughput: {concurrency/duration:.1f} req/s")

def main():
    print("🚀 Advanced Web Server Load Tester")
    print("="*50)
    
    # Check if server is running
    try:
        response = requests.get("http://localhost:8080", timeout=5)
        print(" Server is running and responding")
    except:
        print(" Server is not responding. Please start the server first:")
        print("   make run")
        return
    
    tester = LoadTester()
    
    # Run different types of tests
    print("\n1. Basic Load Test (50 requests, 10 threads)")
    tester.clear_results()
    tester.run_load_test(num_requests=50, num_threads=10)
    
    print("\n2. Cache Effectiveness Test")
    tester.clear_results()
    tester.cache_test()
    
    print("\n3. Concurrent Request Test")
    tester.clear_results()
    tester.concurrent_test()
    
    print("\n4. High Load Test (200 requests, 20 threads)")
    tester.clear_results()
    tester.run_load_test(num_requests=200, num_threads=20)
    
    print("\n All tests completed!")
    print("\nTo view live metrics, visit: http://localhost:8080/metrics")

if __name__ == "__main__":
    main()
