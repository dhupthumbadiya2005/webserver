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
#include <limits.h>
#include <sys/select.h>

#define LB_PORT 8085
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
void *health_check_thread(void *arg);
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
        printf("Failed to connect");
        close(sock);
        return -1;
    }
    
    return sock;
}

void proxy_data(int client_sock, int backend_sock) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    fd_set read_fds;
    int max_fd = (client_sock > backend_sock) ? client_sock : backend_sock;
    
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(client_sock, &read_fds);
        FD_SET(backend_sock, &read_fds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);
        if (activity < 0) {
            if (errno == EINTR) continue;
            perror("Select error");
            break;
        }
        
        if (activity == 0) continue; // Timeout
        
        // Forward data from client to backend
        if (FD_ISSET(client_sock, &read_fds)) {
            bytes_read = recv(client_sock, buffer, BUFFER_SIZE, 0);
            if (bytes_read <= 0) break;
            
            if (send(backend_sock, buffer, bytes_read, 0) <= 0) {
                break;
            }
        }
        
        // Forward data from backend to client
        if (FD_ISSET(backend_sock, &read_fds)) {
            bytes_read = recv(backend_sock, buffer, BUFFER_SIZE, 0);
            if (bytes_read <= 0) break;
            
            if (send(client_sock, buffer, bytes_read, 0) <= 0) {
                break;
            }
        }
    }
}

void health_check_backends() {
    printf("Performing health check on backends...\n");
    
    for (int i = 0; i < num_backends; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            backends[i].active = 0;
            continue;
        }
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(backends[i].port);
        inet_pton(AF_INET, backends[i].host, &addr.sin_addr);
        
        // Set short timeout for health check
        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        int result = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        close(sock);
        
        pthread_mutex_lock(&backend_mutex);
        backends[i].active = (result == 0) ? 1 : 0;
        pthread_mutex_unlock(&backend_mutex);
        
        printf("Backend %s:%d is %s\n", backends[i].host, backends[i].port, 
               backends[i].active ? "UP" : "DOWN");
    }
}

void print_backend_stats() {
    pthread_mutex_lock(&backend_mutex);
    printf("\n=== Backend Statistics ===\n");
    for (int i = 0; i < num_backends; i++) {
        printf("Backend %d: %s:%d - %s - Requests: %ld\n", 
               i, backends[i].host, backends[i].port,
               backends[i].active ? "ACTIVE" : "INACTIVE",
               backends[i].request_count);
    }
    printf("========================\n\n");
    pthread_mutex_unlock(&backend_mutex);
}

void *health_check_thread(void *arg) {
    (void)arg; // Suppress unused parameter warning
    
    while (lb_running) {
        sleep(10); // Health check every 10 seconds
        if (lb_running) {
            health_check_backends();
            print_backend_stats();
        }
    }
    
    return NULL;
}

void *handle_client_lb(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    
    // Select backend using round robin
    int backend_idx = select_backend_round_robin();
    
    if (backend_idx < 0) {
        printf("No active backends available\n");
        const char *error_response = 
            "HTTP/1.1 503 Service Unavailable\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 50\r\n\r\n"
            "<html><body><h1>503 Service Unavailable</h1></body></html>";
        send(client_sock, error_response, strlen(error_response), 0);
        close(client_sock);
        return NULL;
    }
    
    printf("Selected backend %d (%s:%d) for client %d\n", 
           backend_idx, backends[backend_idx].host, backends[backend_idx].port, client_sock);
    
    // Connect to selected backend
    int backend_sock = connect_to_backend(&backends[backend_idx]);
    if (backend_sock < 0) {
        printf("Failed to connect to backend %d\n", backend_idx);
        const char *error_response = 
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 50\r\n\r\n"
            "<html><body><h1>502 Bad Gateway</h1></body></html>";
        send(client_sock, error_response, strlen(error_response), 0);
        close(client_sock);
        return NULL;
    }
    
    // Proxy data between client and backend
    proxy_data(client_sock, backend_sock);
    
    // Cleanup
    close(backend_sock);
    close(client_sock);
    
    printf("Client %d disconnected\n", client_sock);
    return NULL;
}

void signal_handler(int signum) {
    printf("\nReceived signal %d, shutting down load balancer...\n", signum);
    lb_running = 0;
}

int main() {
    int lb_fd, client_sock;
    struct sockaddr_in lb_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t health_check_tid;
    
    printf("Starting Load Balancer on port %d\n", LB_PORT);
    printf("Backend servers:\n");
    for (int i = 0; i < num_backends; i++) {
        printf("  %d: %s:%d\n", i, backends[i].host, backends[i].port);
    }
    printf("\n");
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Create socket
    lb_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (lb_fd < 0) {
        perror("Load balancer socket creation failed");
        exit(1);
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(lb_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(lb_fd);
        exit(1);
    }
    
    // Bind socket
    lb_addr.sin_family = AF_INET;
    lb_addr.sin_addr.s_addr = INADDR_ANY;
    lb_addr.sin_port = htons(LB_PORT);
    
    if (bind(lb_fd, (struct sockaddr *)&lb_addr, sizeof(lb_addr)) < 0) {
        perror("Load balancer bind failed");
        close(lb_fd);
        exit(1);
    }
    
    // Listen for connections
    if (listen(lb_fd, 100) < 0) {
        perror("Load balancer listen failed");
        close(lb_fd);
        exit(1);
    }
    
    printf("Load balancer listening on port %d...\n", LB_PORT);
    
    // Perform initial health check
    health_check_backends();
    
    // Start health check thread
    if (pthread_create(&health_check_tid, NULL, health_check_thread, NULL) != 0) {
        perror("Failed to create health check thread");
    }
    
    // Accept connections
    while (lb_running) {
        client_sock = accept(lb_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_sock < 0) {
            if (lb_running) {
                perror("Accept failed");
            }
            continue;
        }
        
        printf("New client connected: %s:%d (socket %d)\n", 
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_sock);
        
        // Create thread for client
        pthread_t client_thread;
        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_sock;
        
        if (pthread_create(&client_thread, NULL, handle_client_lb, client_sock_ptr) != 0) {
            perror("Failed to create client thread");
            close(client_sock);
            free(client_sock_ptr);
        } else {
            pthread_detach(client_thread);
        }
    }
    
    // Cleanup
    printf("Shutting down load balancer...\n");
    close(lb_fd);
    
    // Cancel health check thread
    pthread_cancel(health_check_tid);
    pthread_join(health_check_tid, NULL);
    
    printf("Load balancer shutdown complete\n");
    return 0;
}