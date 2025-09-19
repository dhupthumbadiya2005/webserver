#include "server.h"

void handle_client(int client_sock) {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    
    char buffer[BUFFER_SIZE];
    char method[16], path[256], protocol[16];
    
    // Read HTTP request
    int bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        gettimeofday(&end_time, NULL);
        double response_time = get_time_diff(start_time, end_time);
        record_request(0, response_time);
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    // Parse HTTP request line
    if (sscanf(buffer, "%s %s %s", method, path, protocol) != 3) {
        send_500(client_sock);
        gettimeofday(&end_time, NULL);
        double response_time = get_time_diff(start_time, end_time);
        record_request(0, response_time);
        return;
    }
    
    printf("Request: %s %s %s\n", method, path, protocol);
    
    // Handle special metrics endpoint
    if (strcmp(path, "/metrics") == 0) {
        pthread_mutex_lock(&metrics_mutex);
        
        char metrics_body[1024];
        double avg_response_time = 0.0;
        double cache_hit_rate = 0.0;
        
        if (total_requests > 0) {
            avg_response_time = total_response_time / total_requests;
            cache_hit_rate = ((double)cache_hits / total_requests) * 100.0;
        }
        
        snprintf(metrics_body, sizeof(metrics_body),
            "<!DOCTYPE html>\n"
            "<html><head><title>Server Metrics</title></head><body>\n"
            "<h1>Server Performance Metrics</h1>\n"
            "<p><strong>Total Requests:</strong> %ld</p>\n"
            "<p><strong>Cache Hits:</strong> %ld</p>\n"
            "<p><strong>Cache Misses:</strong> %ld</p>\n"
            "<p><strong>Cache Hit Rate:</strong> %.2f%%</p>\n"
            "<p><strong>Average Response Time:</strong> %.2f ms</p>\n"
            "<p><strong>Cache Size:</strong> %d entries</p>\n"
            "<p><em>Auto-refresh every 5 seconds</em></p>\n"
            "<script>setTimeout(function(){location.reload();}, 5000);</script>\n"
            "</body></html>",
            total_requests, cache_hits, cache_misses, cache_hit_rate,
            avg_response_time * 1000, cache_size);
        
        pthread_mutex_unlock(&metrics_mutex);
        
        send_response(client_sock, "200 OK", "text/html", metrics_body, strlen(metrics_body));
        
        gettimeofday(&end_time, NULL);
        double response_time = get_time_diff(start_time, end_time);
        record_request(0, response_time);
        return;
    }
    
    // Only handle GET requests for files
    if (strcmp(method, "GET") != 0) {
        send_404(client_sock);
        gettimeofday(&end_time, NULL);
        double response_time = get_time_diff(start_time, end_time);
        record_request(0, response_time);
        return;
    }
    
    // Remove leading slash and handle root path
    char filename[MAX_FILENAME];
    if (strcmp(path, "/") == 0) {
        strcpy(filename, "index.html");
    } else {
        strcpy(filename, path + 1); // Remove leading slash
    }
    
    // Security: prevent directory traversal
    if (strstr(filename, "..") != NULL) {
        send_404(client_sock);
        gettimeofday(&end_time, NULL);
        double response_time = get_time_diff(start_time, end_time);
        record_request(0, response_time);
        return;
    }
    
    int cache_hit = 0;
    char *file_content = NULL;
    size_t file_size = 0;
    
    // Try to get from cache first
    CacheEntry *cached = get_from_cache(filename);
    if (cached) {
        file_content = cached->content;
        file_size = cached->size;
        cache_hit = 1;
        printf("Cache HIT for %s\n", filename);
    } else {
        printf("Cache MISS for %s\n", filename);
        
        // Read file from disk
        FILE *file = fopen(filename, "rb");
        if (!file) {
            send_404(client_sock);
            gettimeofday(&end_time, NULL);
            double response_time = get_time_diff(start_time, end_time);
            record_request(0, response_time);
            return;
        }
        
        // Get file size
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        // Read file content
        char *buffer = malloc(file_size);
        if (!buffer) {
            fclose(file);
            send_500(client_sock);
            gettimeofday(&end_time, NULL);
            double response_time = get_time_diff(start_time, end_time);
            record_request(0, response_time);
            return;
        }
        
        fread(buffer, 1, file_size, file);
        fclose(file);
        
        // Add to cache
        add_to_cache(filename, buffer, file_size);
        file_content = buffer;
    }
    
    // Send response
    char *content_type = get_content_type(filename);
    send_response(client_sock, "200 OK", content_type, file_content, file_size);
    
    // Free allocated memory if not from cache
    if (!cache_hit && file_content) {
        free(file_content);
    }
    
    gettimeofday(&end_time, NULL);
    double response_time = get_time_diff(start_time, end_time);
    record_request(cache_hit, response_time);
}

void send_response(int client_sock, const char *status, const char *content_type, 
                   const char *body, size_t body_size) {
    char header[1024];
    snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "Server: Advanced-Multithreaded-Server/1.0\r\n"
        "\r\n",
        status, content_type, body_size);
    
    send(client_sock, header, strlen(header), 0);
    send(client_sock, body, body_size, 0);
}

void send_404(int client_sock) {
    const char *body = "<!DOCTYPE html><html><body><h1>404 Not Found</h1></body></html>";
    send_response(client_sock, "404 Not Found", "text/html", body, strlen(body));
}

void send_500(int client_sock) {
    const char *body = "<!DOCTYPE html><html><body><h1>500 Internal Server Error</h1></body></html>";
    send_response(client_sock, "500 Internal Server Error", "text/html", body, strlen(body));
}

char *get_content_type(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";
    
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    
    return "application/octet-stream";
}