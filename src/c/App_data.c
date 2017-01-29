#include "App_data.h"
#include "Utility.h"
#include "List.h"
#include "Settings.h"
#include "Timer.h"
#include "globals.h"
#include "assert.h"
#include "Timer_group.h"
#include "persist_util.h"

#include <pebble.h>

static struct App_data* app_data_create();

struct App_data {
  struct List* timer_groups; // List of Lists
  struct Settings* settings;
};

static struct App_data* app_data_create() {
	struct App_data* app_data = safe_alloc(sizeof(struct App_data));
  app_data->timer_groups = list_create();
  app_data->settings = settings_create();
  return app_data;
}

void app_data_destroy(struct App_data* app_data) {
  assert(app_data);
  list_apply(app_data->timer_groups, (List_apply_fp_t)timer_group_destroy);
  list_destroy(app_data->timer_groups);
  app_data->timer_groups = NULL;
  settings_destroy(app_data->settings);
  app_data->settings = NULL;
  free(app_data);
}

struct App_data* app_data_load() {
  persist_init_load();
  if (!persist_exists(PERSIST_VERSION_KEY)) {
    APP_LOG(APP_LOG_LEVEL_INFO, "No data saved, creating new data with default values");
    persist_write_int(PERSIST_VERSION_KEY, PERSIST_VERSION);
    return app_data_create();
  }
  if (persist_read_int(PERSIST_VERSION_KEY) != PERSIST_VERSION) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Persist version changed from %d to %d, resetting data",
      (int) persist_read_int(PERSIST_VERSION_KEY), PERSIST_VERSION);
    persist_write_int(PERSIST_VERSION_KEY, PERSIST_VERSION);
    return app_data_create();
  }
  struct App_data* app_data = safe_alloc(sizeof(struct App_data));
  app_data->timer_groups = list_load((List_load_item_fp_t) timer_group_load);
  app_data->settings = settings_load();
  persist_finish_load();
  return app_data;
}

void app_data_save(struct App_data* app_data) {
  assert(app_data);
  persist_init_save();
  list_save(app_data->timer_groups, (List_apply_fp_t) timer_group_save);
  settings_save(app_data->settings);
  persist_finish_save();
}

struct List* app_data_get_timer_groups(struct App_data* app_data) {
  assert(app_data);
  return app_data->timer_groups;
}

struct Settings* app_data_get_settings(struct App_data* app_data) {
  assert(app_data);
  return app_data->settings;
}

struct Timer_group* app_data_get_timer_group(struct App_data* app_data, int timer_group_index) {
  assert(app_data);
  return list_get(app_data->timer_groups, timer_group_index);
}

struct Timer* app_data_get_timer(struct App_data* app_data, int timer_group_index, int timer_index) { 
  assert(app_data);
  struct Timer_group* timer_group = app_data_get_timer_group(app_data, timer_group_index);
  assert(timer_group);
  return timer_group_get_timer(timer_group, timer_index);
}
