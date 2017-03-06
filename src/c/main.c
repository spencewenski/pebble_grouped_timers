#include "main_window.h"
#include "App_data.h"

#include <pebble.h>

static void init();
static void deinit();

int main()
{
  // persist_delete(PERSIST_VERSION_KEY);
  init();
  app_event_loop();
  deinit();
}

static void init()
{
  main_window_push();
}

static void deinit()
{
  app_data_destroy();
  // persist_delete(PERSIST_VERSION_KEY);
}
