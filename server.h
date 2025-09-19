#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

// Configuration constants
#define PORT 8080
#define MAX_THREADS 10
#define MAX_QUEUE 100
#define BUFFER_SIZE 4096
#define MAX_FILENAME 256
#define MAX_CACHE_SIZE 50
#define METRICS_INTERVAL 10

// Cache entry structure
typedef struct CacheEntry {
    char filename[MAX_FILENAME];
    char *content;
    size_t size;
    time_t last_accessed;
    struct CacheEntry *prev;
    struct CacheEntry *next;
} CacheEntry;

// Global variables
extern int task_queue[MAX_QUEUE];
extern int front, rear, count;
extern pthread_mutex_t queue_mutex;
extern pthread_cond_t queue_not_empty;

extern CacheEntry *cache_head;
extern CacheEntry *cache_tail;
extern int cache_size;
extern pthread_mutex_t cache_mutex;

extern long total_requests;
extern long cache_hits;
extern long cache_misses;
extern double total_response_time;
extern pthread_mutex_t metrics_mutex;

extern int server_running;

// Function prototypes
void enqueue(int client_sock);
int dequeue();
void *worker(void *arg);
void handle_client(int client_sock);
void send_response(int client_sock, const char *status, const char *content_type, 
                   const char *body, size_t body_size);
void send_404(int client_sock);
void send_500(int client_sock);
char *get_content_type(const char *filename);

// Cache functions
CacheEntry* get_from_cache(const char *filename);
void add_to_cache(const char *filename, const char *data, size_t size);
void remove_lru_entry();
void move_to_front(CacheEntry *entry);

// Metrics functions
void record_request(int cache_hit, double response_time);
void *metrics_thread(void *arg);
void print_metrics();

// Utility functions
double get_time_diff(struct timeval start, struct timeval end);
void cleanup_server();
void signal_handler(int signum);

#endif