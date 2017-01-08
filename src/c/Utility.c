#include "Utility.h"

#include <pebble.h>

void* safe_alloc(int size) {
  void* ptr = malloc(size);
  if (!ptr) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed memory allocation of size %d", size);
  }
  return ptr;
}

int in_range(int value, int min, int max) {
  if (value < min || value >= max) {
    return 0;
  }
  return 1;
}