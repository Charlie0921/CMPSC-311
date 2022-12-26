#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "cache.h"
#include "jbod.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int num_queries = 0;
static int num_hits = 0;

static int isCache = 0;

int cache_create(int num_entries) {
  if ((num_entries < 2)||(num_entries > 4096)){ //check if num_entries argument is 2 at minimum and 4096 at maximum
    return -1;
  } else if (isCache == 1) { //check if cache is already created
    return -1;
  } else {
  cache = (cache_entry_t*) calloc(num_entries, sizeof(cache_entry_t)); //allocating space for num_entries cache entries
  cache_size = num_entries;
  isCache = 1;
  return 1;
  }
}

int cache_destroy(void) {
  if (isCache == 0) { //check if cache is already destroyed
    return -1;
  } else {
    free(cache); //freeing dynamically allocated space for cache
    cache = NULL;
    cache_size = 0;
    isCache = 0;
    return 1;
  }
}

int isEmpty = 1;

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  num_queries += 1;
  if (isCache == 0){ //check if the cache is invalid
    return -1;
  } else if (buf == NULL) { //check if the buffer is NULL
    return -1;
  }

  //check if the cache is empty
  for (int i = 0; i < cache_size; i++) {
    if (cache[i].valid == 1){
      isEmpty = 0;
    }
  }
  if (isEmpty == 1){
    return -1;
  }

  //When lookup performs
  for (int i = 0; i < cache_size; i++) {
    if((cache[i].disk_num == disk_num )&&(cache[i].block_num == block_num)) { //if block is identified by disk_num and block_num, copy the block into buf
      memcpy(buf,cache[i].block,256);
      num_hits += 1;
      cache[i].num_accesses += 1;
      isEmpty = 1;
      return 1;
    }
  }
  isEmpty = 1;
  return -1;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  for (int i = 0; i < cache_size; i++){
    if ((cache[i].valid == 1) && ((cache[i].disk_num == disk_num) && (cache[i].block_num == block_num))){ // if entry exists in cache, updates the block content with new data
      memcpy(cache[i].block,buf,256);
      cache[i].num_accesses += 1;
    }
  }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  if (isCache == 0) { //check if cache is uninitialized
    return -1;
  } else if (buf == NULL) { //check if the buffer is NULL
    return -1;
  } else if (disk_num < 0 || disk_num > 16) { //check if the disk number is invalid
    return -1;
  } else if (block_num < 0 || block_num > 256) { //check if the block number is invalid
    return -1;
  }

  for (int i = 0; i < cache_size; i++){
    if(cache[i].valid == 0) { //if indexed cache is valid, insert the information
      cache[i].disk_num = disk_num;
      cache[i].block_num = block_num;
      memcpy(cache[i].block,buf,256);
      cache[i].valid = 1;
      cache[i].num_accesses = 1;
      return 1;
    } else if ((cache[i].valid == 1) && ((cache[i].disk_num == disk_num) && (cache[i].block_num == block_num))){ //if there is already an existin entry in the cache
      return -1;
    }
  }
  //if the cache is full, use LFU policy to overwrite
  int current_accesses = cache[0].num_accesses;
  int index = 0;
  for (int i = 0; i < cache_size; i++){ //count the lowest number of access
    if(cache[i].num_accesses < current_accesses) {
      current_accesses = cache[i].num_accesses;
      index = i;
    }
  }

  //after finding lowest number of access, insert the block information
  cache[index].disk_num = disk_num;
  cache[index].block_num = block_num;
  memcpy(cache[index].block,buf,256);
  cache[index].valid = 1;
  cache[index].num_accesses = 1;
  return 1;
}

bool cache_enabled(void) {
	return cache != NULL && cache_size > 0;
}

void cache_print_hit_rate(void) {
	fprintf(stderr, "num_hits: %d, num_queries: %d\n", num_hits, num_queries);
	fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
