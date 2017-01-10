#include "timer_edit_window.h"
#include "App_data.h"
#include "globals.h"

#include <pebble.h>

static Window* s_timer_edit_window;

// WindowHandlers
static void window_load_handler(Window* window);
static void window_unload_handler(Window* window);

void timer_edit_window_push(struct App_data* app_data) {
  s_timer_edit_window = window_create();
  
  if (!s_timer_edit_window) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null main window");
    return;
  }
  
  window_set_user_data(s_timer_edit_window, app_data);
  
  window_set_window_handlers(s_timer_edit_window, (WindowHandlers) {
    .load = window_load_handler,
    .unload = window_unload_handler
  });
  
  window_stack_push(s_timer_edit_window, false);
}

static void window_load_handler(Window* window) {
  
}

static void window_unload_handler(Window* window) {
  struct App_data* app_data = window_get_user_data(window);
  app_data_set_current_timer_index(app_data, INVALID_INDEX);

  window_destroy(s_timer_edit_window);
  s_timer_edit_window = NULL;
}