#include "persist_util.h"
#include "assert.h"

#include <pebble.h>

#define INVALID_PERSIST_KEY -1

#define MAX_PERSIST_KEY_KEY 1
static int g_max_persist_key = 0;

int g_current_persist_key = 0;

void persist_init_load()
{
  g_current_persist_key = MAX_PERSIST_KEY_KEY + 1;
  if (!persist_exists(MAX_PERSIST_KEY_KEY)) {
    g_max_persist_key = INVALID_PERSIST_KEY;
  } else {
    g_max_persist_key = persist_read_int(MAX_PERSIST_KEY_KEY);
  }
}

void persist_init_save()
{
  g_current_persist_key = MAX_PERSIST_KEY_KEY + 1;
}

void persist_finish_load()
{
  assert(g_max_persist_key == INVALID_PERSIST_KEY || g_current_persist_key == g_max_persist_key);
}

void persist_finish_save()
{
  g_max_persist_key = g_current_persist_key;
  persist_write_int(MAX_PERSIST_KEY_KEY, g_max_persist_key);
}