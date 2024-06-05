#include "lru_cache.h"


struct cache_element* head = NULL;
int cache_size             = 0;


char* cache_refer(char* url, ssize_t* res_data_nbytes) {
    struct cache_element* curr = head;

    // go thru the cache
    while(curr) {
        // check if url is already in cache
        if(strcmp(curr->url, url) == 0) {
            // if expired
            if(time(NULL) - curr->timestamp >= CACHE_TTL_SEC) {
                // delete the entry
                if(curr->prev) { curr->prev->next = curr->next; }  // if not first entry
                else { head = curr->next; }  // if first entry
                free(curr->url);
                free(curr->res_data);
                free(curr);
                --cache_size;

                // and return NULL
                return NULL;
            } else {
                // else, not expired
                // then, if not the first element
                if(curr->prev) {
                    // shift the entry to the front (most recently used)
                    curr->prev->next = curr->next;
                    curr->next = head;
                    curr->prev = NULL;
                    head = curr;
                }

                
                // set the res_data_nbytes
                *res_data_nbytes = curr->res_data_nbytes;

                // and return the cached response data
                return curr->res_data;
            }
        }

        curr = curr->next;
    }


    // return NULL if not cached
    return NULL;
}

int cache_upsert(char* url, char* res_data, ssize_t* res_data_nbytes) {
    int updated = 0;  // set to 1 if 

    // go thru the cache
    struct cache_element* curr = head;
    while(curr) {
        // check if url is already in cache
        if(strcmp(curr->url, url) == 0) {
            // delete the duplicate entry
            if(curr->prev) { curr->prev->next = curr->next; }  // if not first entry
            else { head = curr->next; }  // if first entry
            free(curr->url);
            free(curr->res_data);
            free(curr);
            --cache_size;

            updated = 1;
        }

        curr = curr->next;
    }

    // proceed to add the new/updated entry
    // firstly, check if the cache is full before storing (useful when storing an unique entry)
    if(cache_size >= MAX_CACHE_SIZE) {
        // then delete the last element (least recently used)
        struct cache_element* curr = head;
        while(curr) {
            if(curr->next == NULL) {
                curr->prev->next = curr->next;

                free(curr->url);
                free(curr->res_data);
                free(curr);

                --cache_size;

                break;
            }
        }
    }


    // alloc the cache element
    struct cache_element* e = (struct cache_element*)malloc(sizeof(struct cache_element));
    
    // load the url
    e->url = malloc(strlen(url) + 1);
    memset(e->url, 0, strlen(url) + 1);
    strcpy(e->url, url);

    // load the response
    e->res_data_nbytes = *res_data_nbytes;
    e->res_data        =  malloc(*res_data_nbytes);
    memset(e->res_data, 0, (*res_data_nbytes) + 1);
    memcpy(e->res_data, res_data, *res_data_nbytes);

    // set the timestamp
    e->timestamp = time(NULL);

    // emplace it on the doubly-linked-list
    e->prev = NULL;
    e->next = head;
    head = e;

    ++cache_size;

    return updated;
}

