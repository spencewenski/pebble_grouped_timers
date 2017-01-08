#include "Timer.h"
#include "App_data.h"
#include "main_window.h"

#include <pebble.h>

static void init(struct App_data* app_data) {
  main_window_push(app_data);
}

static void deinit() {
//   main_window_deinit();
}

int main() {
  struct App_data* app_data = app_data_create();
  init(app_data);
  app_event_loop();
  deinit();
  app_data_destroy(app_data);
}