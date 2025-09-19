#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define LB_PORT 8080
#define BUFFER_SIZE 4096
#define MAX_BACKENDS 4

// Backend server configuration
typedef struct {
    char host[64];
    int port;
    int active;
    long request_count;
} Backend;

// Global variables
Backend backends[] = {
    {"127.0.0.1", 8081, 1, 0},
    {"127.0.0.1", 8082, 1, 0},
    {"127.0.0.1", 8083, 1, 0},
    {"127.0.0.1", 8084, 1, 0}
};

int num_backends = 4;
int current_backend = 0;
int lb_running = 1;
pthread_mutex_t backend_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
int select_backend_round_robin();
int select_backend_least_connections();
void *handle_client_lb(void *arg);
int connect_to_backend(Backend *backend);
void proxy_data(int client_sock, int backend_sock);
void health_check_backends();
void signal_handler(int signum);
void print_backend_stats();

int select_backend_round_robin() {
    pthread_mutex_lock(&backend_mutex);
    
    int selected = -1;
    int attempts = 0;
    
    // Try to find an active backend using round robin
    while (attempts < num_backends) {
        if (backends[current_backend].active) {
            selected = current_backend;
            backends[current_backend].request_count++;
            current_backend = (current_backend + 1) % num_backends;
            break;
        }
        current_backend = (current_backend + 1) % num_backends;
        attempts++;
    }
    
    pthread_mutex_unlock(&backend_mutex);
    return selected;
}

int select_backend_least_connections() {
    pthread_mutex_lock(&backend_mutex);
    
    int selected = -1;
    long min_connections = LONG_MAX;
    
    for (int i = 0; i < num_backends; i++) {
        if (backends[i].active && backends[i].request_count < min_connections) {
            min_connections = backends[i].request_count;
            selected = i;
        }
    }
    
    if (selected >= 0) {
        backends[selected].request_count++;
    }
    
    pthread_mutex_unlock(&backend_mutex);
    return selected;
}

int connect_to_backend(Backend *backend) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Backend socket creation failed");
        return -1;
    }
    
    struct sockaddr_in backend_addr;
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(backend->port);
    
    if (inet_pton(AF_INET, backend->host, &backend_addr.sin_addr) <= 0) {
        perror("Invalid backend address");
        close(sock);
        return -1;
    }
    
    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    if (connect(sock, (struct sockaddr *)&backend_addr, sizeof(backend_addr)) < 0) {
        printf("Failed to connect