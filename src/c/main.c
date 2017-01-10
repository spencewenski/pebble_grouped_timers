#include "Timer.h"
#include "App_data.h"
#include "main_window.h"
#include "List.h"

#include <pebble.h>

static void init(struct App_data* app_data);
static void deinit();

static void init_fake_data(struct App_data* app_data);

int main() {
  struct App_data* app_data = app_data_create();
  init(app_data);
  app_event_loop();
  deinit(app_data);
}

static void init(struct App_data* app_data) {
  init_fake_data(app_data);
  main_window_push(app_data);
}

static void init_fake_data(struct App_data* app_data) {
  // Timer 1
  struct List* timer_groups = app_data_get_timer_groups(app_data);
  list_add(timer_groups, list_create());
  struct List* timer_group = list_get(timer_groups, 0);
  
  struct Timer* timer = timer_create();
  timer_set_all(timer, 1, 0, 10);
  list_add(timer_group, timer);
  
  timer = timer_create();
  timer_set_all(timer, 0, 1, 30);
  list_add(timer_group, timer);
  
  // Timer 2
  list_add(timer_groups, list_create());
  timer_group = list_get(timer_groups, 1);
  
  timer = timer_create();
  timer_set_all(timer, 0, 0, 15);
  list_add(timer_group, timer);
}

static void deinit(struct App_data* app_data) {
  app_data_destroy(app_data);
}