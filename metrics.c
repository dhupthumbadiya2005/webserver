#include "server.h"

// Global metrics variables
long total_requests = 0;
long cache_hits = 0;
long cache_misses = 0;
double total_response_time = 0.0;
pthread_mutex_t metrics_mutex = PTHREAD_MUTEX_INITIALIZER;

void record_request(int cache_hit, double response_time) {
    pthread_mutex_lock(&metrics_mutex);
    
    total_requests++;
    total_response_time += response_time;
    
    if (cache_hit) {
        cache_hits++;
    } else {
        cache_misses++;
    }
    
    pthread_mutex_unlock(&metrics_mutex);
}

void print_metrics() {
    pthread_mutex_lock(&metrics_mutex);
    
    double avg_response_time = 0.0;
    double cache_hit_rate = 0.0;
    
    if (total_requests > 0) {
        avg_response_time = total_response_time / total_requests;
        cache_hit_rate = ((double)cache_hits / total_requests) * 100.0;
    }
    
    printf("\n=== SERVER METRICS ===\n");
    printf("Total Requests: %ld\n", total_requests);
    printf("Cache Hits: %ld\n", cache_hits);
    printf("Cache Misses: %ld\n", cache_misses);
    printf("Cache Hit Rate: %.2f%%\n", cache_hit_rate);
    printf("Average Response Time: %.2f ms\n", avg_response_time * 1000);
    printf("Cache Size: %d entries\n", cache_size);
    printf("=======================\n\n");
    
    pthread_mutex_unlock(&metrics_mutex);
}

void *metrics_thread(void *arg) {
    printf("Metrics thread started\n");
    
    while (server_running) {
        sleep(METRICS_INTERVAL);
        if (server_running) {
            print_metrics();
        }
    }
    
    printf("Metrics thread stopping\n");
    pthread_exit(NULL);
}

double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}