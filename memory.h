#include <stdlib.h>

typedef struct Allocator {
  char* data;
  size_t size, count, capacity;
} Allocator;

Allocator* allocator_new(size_t size, size_t capacity) {
  Allocator* result = malloc(sizeof(Allocator));
  result->data = malloc(size * capacity);
  result->size = size;
  result->count = 0;
  result->capacity = capacity;
  return result;
}

void allocator_free(Allocator* allocator) {
  free(allocator->data);
  free(allocator);
}

void allocator_resize(Allocator* allocator, size_t new_capacity) {
  allocator->data = realloc(allocator->data, new_capacity * allocator->size);
  if (allocator->data == NULL)
    log_error("Failed to reallocate memory");
  allocator->capacity = new_capacity;
}

void* allocator_get(Allocator* allocator, size_t count) {
  if (allocator->count + count > allocator->capacity)
    allocator_resize(allocator, allocator->capacity * 2);
  allocator->count += count;
  return (void*)&allocator->data[(allocator->count - count) * allocator->size];
}
