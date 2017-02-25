#include "Utility.h"
#include "assert.h"

#include <pebble.h>

void* safe_alloc(int size)
{
  void* ptr = malloc(size);
  assert(ptr);
  return ptr;
}

int in_range(int value, int min, int max)
{
  if (value < min || value >= max) {
    return 0;
  }
  return 1;
}

int wrap_value(int value, int min, int max)
{
  if (value < min) {
    value += max;
  }
  return value % max;
}

int min(int a, int b)
{
  return a < b ? a : b;
}

int max(int a, int b)
{
  return a > b ? a : b;
}
