#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_hashmap.h"

/**
 * Creates a new thread-safe hashmap.
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity)
{
  ts_hashmap_t *map = malloc(sizeof(ts_hashmap_t));
  map->table = malloc(sizeof(ts_entry_t *) * capacity);
  map->locks = malloc(sizeof(pthread_mutex_t) * capacity);

  for (int i = 0; i < capacity; i++)   // Initialize all lists to null
  {
    map->table[i] = NULL;
    pthread_mutex_init(&map->locks[i], NULL);
  }

  map->capacity = capacity;
  map->size = 0;
  map->numOps = 0;

  return map;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key)
{
  int index = ((unsigned int)key) % map->capacity;
  int returnVal;

  pthread_mutex_lock(&map->locks[index]); // Lock up this bucket

  ts_entry_t *entry = map->table[index];

  // Traverse the linked list
  while (entry != NULL)
  {
    if (entry->key == key)
    {
      map->numOps++;
      returnVal = entry->value;
      pthread_mutex_unlock(&map->locks[index]); // Unlock the bucket after we have the value
      return returnVal;
    }
    entry = entry->next;
  }

  map->numOps++;
  pthread_mutex_unlock(&map->locks[index]); // Unlock the bucket after searching is finished
  return INT_MAX; // Key not found
}

/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value)
{
  int index = ((unsigned int)key) % map->capacity;
  pthread_mutex_lock(&map->locks[index]); // Lock up this bucket
  ts_entry_t *entry = map->table[index];

  while (entry != NULL) // Traverse the linked list
  {
    if (entry->key == key) // Key exists, replace the value
    {
      int temp = entry->value;
      entry->value = value;
      map->numOps++;
      pthread_mutex_unlock(&map->locks[index]); // unlock this bucket
      return temp;
    }
    if (entry->next == NULL)
    { // Need this to handle case where list isn't empty (I learned the hard way)
      break;
    }
    entry = entry->next;
  }

  // Key not found, create a new entry
  ts_entry_t *entry2 = malloc(sizeof(ts_entry_t));
  entry2->key = key;
  entry2->value = value;
  entry2->next = NULL;

  if (entry == NULL)
  {
    map->table[index] = entry2; // Empty list, this is the first element
  }
  else
  {
    entry->next = entry2; // We're adding to this linked list
  }

  map->size++;
  map->numOps++;
  pthread_mutex_unlock(&map->locks[index]); // unlock this bucket
  return INT_MAX;
}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key)
{

  int index = ((unsigned int)key) % map->capacity;
  pthread_mutex_lock(&map->locks[index]); // Lock up this bucket
  ts_entry_t *entry = map->table[index];
  ts_entry_t *prev = NULL;

  while (entry != NULL)
  {
    if (entry->key == key) // Found the entry for deletion!
    {
      int temp = entry->value;

      if (prev == NULL) // First item in the list
      {
        map->table[index] = entry->next;
      }
      else // In the middle or end of the list
      {
        prev->next = entry->next;
      }

      free(entry);
      map->size--;
      map->numOps++;
      pthread_mutex_unlock(&map->locks[index]); // Unlock bucket
      return temp;
    }

    prev = entry;
    entry = entry->next;
  }

  // Key not found
  map->numOps++;
  pthread_mutex_unlock(&map->locks[index]); // Unlock bucket
  return INT_MAX;
}

/**
 * Prints the contents of the map (given)
 */
void printmap(ts_hashmap_t *map)
{
  for (int i = 0; i < map->capacity; i++)
  {
    printf("[%d] -> ", i);
    ts_entry_t *entry = map->table[i];
    while (entry != NULL)
    {
      printf("(%d,%d)", entry->key, entry->value);
      if (entry->next != NULL)
        printf(" -> ");
      entry = entry->next;
    }
    printf("\n");
  }
}

/**
 * Free up the space allocated for hashmap
 * @param map a pointer to the map
 */
void freeMap(ts_hashmap_t *map)
{
  // Free each linked list in the table
  for (int i = 0; i < map->capacity; i++)
  {
    ts_entry_t *entry = map->table[i];
    pthread_mutex_destroy(&map->locks[i]);
    while (entry != NULL)
    {
      ts_entry_t *next = entry->next;
      free(entry);
      entry = next;
    }
  }

  free(map->table);
  free(map->locks);
  free(map);
}