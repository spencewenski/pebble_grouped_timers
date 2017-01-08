#include "main_window.h"
#include "App_data.h"

#include <pebble.h>

static Window* main_window_s;

void main_window_push(struct App_data* app_data) {
  main_window_s = window_create();
  
  if (!main_window_s) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null main window");
  }
  
  window_set_user_data(main_window_s, app_data);
  
  window_stack_push(main_window_s, false);
}
