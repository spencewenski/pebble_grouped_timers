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

void exit_app()
{
  window_stack_pop_all(false);
}

void click_handler_exit_app(ClickRecognizerRef recognizer, void* context)
{
  exit_app();
}

void menu_click_handler_exit_app(MenuLayer* menu_layer, MenuIndex* cell_index, void* data)
{
  exit_app();
}
