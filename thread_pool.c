#include "server.h"

// Global thread pool variables (defined here, declared in server.h)
int task_queue[MAX_QUEUE];
int front = 0, rear = 0, count = 0;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

void enqueue(int client_sock) {
    pthread_mutex_lock(&queue_mutex);
    
    // If queue is full, we could either block or drop the request
    // For this implementation, we'll wait for space
    while (count == MAX_QUEUE) {
        printf("Queue is full, waiting...\n");
        pthread_mutex_unlock(&queue_mutex);
        usleep(1000); // Sleep for 1ms
        pthread_mutex_lock(&queue_mutex);
    }
    
    task_queue[rear] = client_sock;
    rear = (rear + 1) % MAX_QUEUE;
    count++;
    
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_mutex);
}

int dequeue() {
    pthread_mutex_lock(&queue_mutex);
    
    while (count == 0) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    
    int sock = task_queue[front];
    front = (front + 1) % MAX_QUEUE;
    count--;
    
    pthread_mutex_unlock(&queue_mutex);
    return sock;
}

void *worker(void *arg) {
    int thread_id = *(int*)arg;
    printf("Worker thread %d started\n", thread_id);
    
    while (server_running) {
        int client_sock = dequeue();
        
        if (!server_running) {
            close(client_sock);
            break;
        }
        
        printf("Thread %d handling client %d\n", thread_id, client_sock);
        handle_client(client_sock);
        close(client_sock);
    }
    
    printf("Worker thread %d stopping\n", thread_id);
    pthread_exit(NULL);
}