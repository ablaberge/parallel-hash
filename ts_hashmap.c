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

  // Initialize all lists to null
  for (int i = 0; i < capacity; i++)
  {
    map->table[i] = NULL;
  }

  map->capacity = capacity;
  map->size = 0;
  map->numOps = 0;

  return map;
}

int get(ts_hashmap_t *map, int key)
{
  // Calculate the index using the hash function
  int index = ((unsigned int)key) % map->capacity;

  // Start at the head of the linked list in this bucket
  ts_entry_t *entry = map->table[index];

  // Traverse the linked list
  while (entry != NULL)
  {
    if (entry->key == key)
    {
      return entry->value;
    }
    entry = entry->next;
  }
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
  ts_entry_t *entry = map->table[index];
  // Traverse the linked list
  while (entry != NULL)
  {
    if (entry->key == key) // Key exists, replace the value
    {
      int temp = entry->value;
      entry->value = value;
      map->numOps++;
      return temp;
    }
    if (entry->next == NULL)
    { // Need this to handle case where list isn't empty
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
      return temp;
    }

    prev = entry;
    entry = entry->next;
  }

 // Key not found
  map->numOps++;
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
    while (entry != NULL)
    {
      ts_entry_t *next = entry->next;
      free(entry);
      entry = next;
    }
  }

  // Free the table array itself
  free(map->table);

  // Free the map struct
  free(map);

  // TODO: destroy locks
}