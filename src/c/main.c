#include "main_window.h"
#include "App_data.h"
#include "Wakeup_manager.h"
#include "persist_util.h"

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
  wakeup_manager_handle_wakeup(app_data_get_wakeup_manager(app_data_get()));
  main_window_push();
}

static void deinit()
{
  app_data_destroy();
  // persist_delete(PERSIST_VERSION_KEY);
}
