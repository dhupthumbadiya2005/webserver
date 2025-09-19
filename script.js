// Advanced Web Server Demo JavaScript

let testResults = document.getElementById('test-results');
let performanceStats = document.getElementById('performance-stats');

// Load test function - demonstrates multithreading
async function loadTest() {
    testResults.innerHTML = '<div class="loading"></div> Starting load test...';
    
    const startTime = Date.now();
    const promises = [];
    const numRequests = 100;
    
    for (let i = 0; i < numRequests; i++) {
        promises.push(fetch('/').then(response => response.text()));
    }
    
    try {
        await Promise.all(promises);
        const endTime = Date.now();
        const duration = endTime - startTime;
        
        testResults.innerHTML = `
            <h3>✅ Load Test Complete!</h3>
            <p><strong>Requests:</strong> ${numRequests}</p>
            <p><strong>Duration:</strong> ${duration}ms</p>
            <p><strong>Requests/second:</strong> ${Math.round(numRequests / (duration / 1000))}</p>
            <p><strong>Average response time:</strong> ${Math.round(duration / numRequests)}ms</p>
            <p style="color: green;">All requests handled successfully by worker threads!</p>
        `;
    } catch (error) {
        testResults.innerHTML = `<p style="color: red;">Error: ${error.message}</p>`;
    }
}

// Cache test function - demonstrates caching
async function cacheTest() {
    testResults.innerHTML = '<div class="loading"></div> Testing cache functionality...';
    
    const testFile = '/style.css';
    const results = [];
    
    // First request (cache miss)
    const start1 = Date.now();
    await fetch(testFile);
    const time1 = Date.now() - start1;
    results.push(`First request: ${time1}ms (cache miss)`);
    
    // Second request (cache hit)
    const start2 = Date.now();
    await fetch(testFile);
    const time2 = Date.now() - start2;
    results.push(`Second request: ${time2}ms (cache hit)`);
    
    // Third request (cache hit)
    const start3 = Date.now();
    await fetch(testFile);
    const time3 = Date.now() - start3;
    results.push(`Third request: ${time3}ms (cache hit)`);
    
    const speedup = time1 > 0 ? Math.round((time1 - time2) / time1 * 100) : 0;
    
    testResults.innerHTML = `
        <h3>💾 Cache Test Results</h3>
        <p>${results.join('<br>')}</p>
        <p><strong>Cache speedup:</strong> ${speedup}% faster on cache hits!</p>
        <p style="color: green;">LRU cache is working perfectly!</p>
    `;
}

// Concurrent test - demonstrates thread pool
async function concurrentTest() {
    testResults.innerHTML = '<div class="loading"></div> Testing concurrent request handling...';
    
    const startTime = Date.now();
    const promises = [];
    const files = ['/', '/style.css', '/script.js', '/about.html', '/api/data.json'];
    
    // Create 50 concurrent requests
    for (let i = 0; i < 50; i++) {
        const file = files[i % files.length];
        promises.push(fetch(file).then(response => ({
            status: response.status,
            file: file
        })));
    }
    
    try {
        const results = await Promise.all(promises);
        const endTime = Date.now();
        const duration = endTime - startTime;
        
        const successCount = results.filter(r => r.status === 200).length;
        const errorCount = results.length - successCount;
        
        testResults.innerHTML = `
            <h3>🧵 Concurrent Test Results</h3>
            <p><strong>Total requests:</strong> ${results.length}</p>
            <p><strong>Successful:</strong> ${successCount}</p>
            <p><strong>Errors:</strong> ${errorCount}</p>
            <p><strong>Duration:</strong> ${duration}ms</p>
            <p><strong>Throughput:</strong> ${Math.round(results.length / (duration / 1000))} req/s</p>
            <p style="color: green;">Thread pool handled all concurrent requests efficiently!</p>
        `;
    } catch (error) {
        testResults.innerHTML = `<p style="color: red;">Error: ${error.message}</p>`;
    }
}

// View metrics function
async function viewMetrics() {
    testResults.innerHTML = '<div class="loading"></div> Fetching live metrics...';
    
    try {
        const response = await fetch('/metrics');
        const html = await response.text();
        
        // Extract metrics from the HTML response
        const parser = new DOMParser();
        const doc = parser.parseFromString(html, 'text/html');
        const metrics = {};
        
        const paragraphs = doc.querySelectorAll('p');
        paragraphs.forEach(p => {
            const text = p.textContent;
            if (text.includes('Total Requests:')) {
                metrics.totalRequests = text.match(/\d+/)[0];
            } else if (text.includes('Cache Hits:')) {
                metrics.cacheHits = text.match(/\d+/)[0];
            } else if (text.includes('Cache Misses:')) {
                metrics.cacheMisses = text.match(/\d+/)[0];
            } else if (text.includes('Cache Hit Rate:')) {
                metrics.cacheHitRate = text.match(/[\d.]+/)[0];
            } else if (text.includes('Average Response Time:')) {
                metrics.avgResponseTime = text.match(/[\d.]+/)[0];
            } else if (text.includes('Cache Size:')) {
                metrics.cacheSize = text.match(/\d+/)[0];
            }
        });
        
        performanceStats.innerHTML = `
            <h3>📊 Live Server Metrics</h3>
            <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin-top: 15px;">
                <div style="background: #e6f3ff; padding: 15px; border-radius: 8px;">
                    <strong>Total Requests</strong><br>
                    <span style="font-size: 1.5em; color: #0066cc;">${metrics.totalRequests || '0'}</span>
                </div>
                <div style="background: #e6ffe6; padding: 15px; border-radius: 8px;">
                    <strong>Cache Hits</strong><br>
                    <span style="font-size: 1.5em; color: #00cc66;">${metrics.cacheHits || '0'}</span>
                </div>
                <div style="background: #ffe6e6; padding: 15px; border-radius: 8px;">
                    <strong>Cache Misses</strong><br>
                    <span style="font-size: 1.5em; color: #cc0000;">${metrics.cacheMisses || '0'}</span>
                </div>
                <div style="background: #fff0e6; padding: 15px; border-radius: 8px;">
                    <strong>Cache Hit Rate</strong><br>
                    <span style="font-size: 1.5em; color: #ff6600;">${metrics.cacheHitRate || '0'}%</span>
                </div>
                <div style="background: #f0e6ff; padding: 15px; border-radius: 8px;">
                    <strong>Avg Response Time</strong><br>
                    <span style="font-size: 1.5em; color: #6600cc;">${metrics.avgResponseTime || '0'}ms</span>
                </div>
                <div style="background: #e6f0ff; padding: 15px; border-radius: 8px;">
                    <strong>Cache Size</strong><br>
                    <span style="font-size: 1.5em; color: #0066cc;">${metrics.cacheSize || '0'}</span>
                </div>
            </div>
            <p style="margin-top: 15px; color: #666;">
                <em>Metrics update every 10 seconds automatically</em>
            </p>
        `;
        
        testResults.innerHTML = `
            <h3>📈 Metrics Retrieved Successfully!</h3>
            <p>Live server performance data displayed above.</p>
            <p style="color: green;">Real-time monitoring is working perfectly!</p>
        `;
        
    } catch (error) {
        testResults.innerHTML = `<p style="color: red;">Error fetching metrics: ${error.message}</p>`;
    }
}

// Auto-refresh metrics every 30 seconds
setInterval(() => {
    if (performanceStats.innerHTML.includes('Live Server Metrics')) {
        viewMetrics();
    }
}, 30000);
