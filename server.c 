#include "server.h"

// Global server state
int server_running = 1;

void cleanup_server() {
    printf("Cleaning up server resources...\n");
    
    // Stop server
    server_running = 0;
    
    // Clean up cache
    pthread_mutex_lock(&cache_mutex);
    CacheEntry *curr = cache_head;
    while (curr) {
        CacheEntry *next = curr->next;
        free(curr->content);
        free(curr);
        curr = next;
    }
    cache_head = cache_tail = NULL;
    cache_size = 0;
    pthread_mutex_unlock(&cache_mutex);
    
    // Wake up all waiting threads
    pthread_cond_broadcast(&queue_not_empty);
    
    printf("Server cleanup completed\n");
}

void signal_handler(int signum) {
    printf("\nReceived signal %d, shutting down gracefully...\n", signum);
    cleanup_server();
    exit(0);
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t worker_threads[MAX_THREADS];
    pthread_t metrics_tid;
    int thread_ids[MAX_THREADS];
    
    printf("🚀 Starting Advanced Multithreaded Web Server\n");
    printf("Features: Thread Pooling, Caching, Performance Metrics\n");
    printf("Port: %d, Threads: %d, Cache Size: %d\n\n", PORT, MAX_THREADS, MAX_CACHE_SIZE);
    
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(1);
    }
    
    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }
    
    // Listen for connections
    if (listen(server_fd, MAX_QUEUE) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    // Create worker threads
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&worker_threads[i], NULL, worker, &thread_ids[i]) != 0) {
            perror("Failed to create worker thread");
            cleanup_server();
            close(server_fd);
            exit(1);
        }
    }
    
    // Create metrics thread
    if (pthread_create(&metrics_tid, NULL, metrics_thread, NULL) != 0) {
        perror("Failed to create metrics thread");
        cleanup_server();
        close(server_fd);
        exit(1);
    }
    
    printf("All worker threads and metrics thread started\n");
    printf("Visit http://localhost:%d/metrics to see performance metrics\n\n", PORT);
    
    // Accept connections
    while (server_running) {
        client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_sock < 0) {
            if (server_running) {
                perror("Accept failed");
            }
            continue;
        }
        
        printf("New client connected: %s:%d (socket %d)\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_sock);
        
        // Add to task queue
        enqueue(client_sock);
    }
    
    // Wait for all threads to finish
    printf("Waiting for worker threads to finish...\n");
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    
    pthread_join(metrics_tid, NULL);
    
    close(server_fd);
    cleanup_server();
    
    printf("Server shutdown complete\n");
    return 0;
}