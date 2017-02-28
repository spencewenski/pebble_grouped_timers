#include "Timer.h"
#include "App_data.h"
#include "main_window.h"
#include "List.h"
#include "Settings.h"
#include "Timer_group.h"
#include "assert.h"
#include "persist_util.h"

#include <pebble.h>

static void init(struct App_data* app_data);
static void deinit();

int main()
{
  // persist_delete(PERSIST_VERSION_KEY);
  struct App_data* app_data = app_data_load();
  init(app_data);
  app_event_loop();
  deinit(app_data);
}

static void init(struct App_data* app_data)
{
  main_window_push(app_data);
}

static void deinit(struct App_data* app_data)
{
  app_data_save(app_data);
  app_data_destroy(app_data);
  // persist_delete(PERSIST_VERSION_KEY);
}
