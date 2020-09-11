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
  void* ptr = realloc(allocator->data, new_capacity * allocator->size);
  if (ptr == NULL)
    log_error("Failed to reallocate memory");
  allocator->data = ptr;
  allocator->capacity = new_capacity;
}

size_t allocator_get(Allocator* allocator, size_t count) {
  if (allocator->count + count > allocator->capacity)
    allocator_resize(allocator, allocator->capacity * 2);
  allocator->count += count;
  return allocator->count - count;
}

void* allocator_at(Allocator* allocator, size_t index) {
  if (index >= allocator->count) {
    log_error("Invalid index");
    return NULL;
  }
  return &allocator->data[index * allocator->size];
}
