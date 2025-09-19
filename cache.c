#include "server.h"

// Global cache variables (defined here, declared in server.h)
CacheEntry *cache_head = NULL;
CacheEntry *cache_tail = NULL;
int cache_size = 0;
pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

CacheEntry* get_from_cache(const char *filename) {
    pthread_mutex_lock(&cache_mutex);
    
    CacheEntry *curr = cache_head;
    while (curr) {
        if (strcmp(curr->filename, filename) == 0) {
            // Update last accessed time
            curr->last_accessed = time(NULL);
            // Move to front (most recently used)
            move_to_front(curr);
            pthread_mutex_unlock(&cache_mutex);
            return curr;
        }
        curr = curr->next;
    }
    
    pthread_mutex_unlock(&cache_mutex);
    return NULL;
}

void add_to_cache(const char *filename, const char *data, size_t size) {
    pthread_mutex_lock(&cache_mutex);
    
    // Check if we need to remove LRU entry
    if (cache_size >= MAX_CACHE_SIZE) {
        remove_lru_entry();
    }
    
    // Create new entry
    CacheEntry *entry = malloc(sizeof(CacheEntry));
    if (!entry) {
        pthread_mutex_unlock(&cache_mutex);
        return;
    }
    
    strcpy(entry->filename, filename);
    entry->content = malloc(size);
    if (!entry->content) {
        free(entry);
        pthread_mutex_unlock(&cache_mutex);
        return;
    }
    
    memcpy(entry->content, data, size);
    entry->size = size;
    entry->last_accessed = time(NULL);
    entry->prev = NULL;
    entry->next = cache_head;
    
    // Insert at head
    if (cache_head) {
        cache_head->prev = entry;
    } else {
        cache_tail = entry;
    }
    cache_head = entry;
    cache_size++;
    
    printf("Added '%s' to cache (size: %zu bytes)\n", filename, size);
    pthread_mutex_unlock(&cache_mutex);
}

void remove_lru_entry() {
    if (!cache_tail) return;
    
    CacheEntry *lru = cache_tail;
    
    if (lru->prev) {
        lru->prev->next = NULL;
        cache_tail = lru->prev;
    } else {
        cache_head = cache_tail = NULL;
    }
    
    printf("Evicting '%s' from cache\n", lru->filename);
    free(lru->content);
    free(lru);
    cache_size--;
}

void move_to_front(CacheEntry *entry) {
    if (entry == cache_head) return;
    
    // Remove from current position
    if (entry->next) {
        entry->next->prev = entry->prev;
    } else {
        cache_tail = entry->prev;
    }
    
    if (entry->prev) {
        entry->prev->next = entry->next;
    }
    
    // Move to front
    entry->prev = NULL;
    entry->next = cache_head;
    if (cache_head) {
        cache_head->prev = entry;
    }
    cache_head = entry;
    
    if (!cache_tail) {
        cache_tail = entry;
    }
}