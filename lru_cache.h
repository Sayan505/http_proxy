#pragma once


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#define MAX_CACHE_SIZE ((int)(16))
#define CACHE_TTL_SEC  ((time_t)(1200))  // 20 mins

#define MAX_CACHE_RES_SIZE ((int)(65536))
struct cache_element {
    char*   url;
    char*   res_data;
    ssize_t res_data_nbytes;

    time_t  timestamp;


    struct cache_element* prev;
    struct cache_element* next;
};


char* cache_refer(char* url, ssize_t* res_data_nbytes);
int   cache_upsert(char* url, char* res_data, ssize_t* res_data_nbytes);

