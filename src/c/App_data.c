#include "App_data.h"
#include "Utility.h"
#include "List.h"
#include "Settings.h"
#include "Timer.h"
#include "globals.h"

#include <pebble.h>

struct App_data {
  struct List* timer_groups; // List of Lists
  struct Settings* settings;
};

struct App_data* app_data_create() {
	struct App_data* app_data = safe_alloc(sizeof(struct App_data));
  app_data->timer_groups = list_create();
  app_data->settings = settings_create();
  return app_data;
}

void app_data_destroy(struct App_data* app_data) {
	if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null app_data pointer");
    return;
  }
  for (int i = 0; i < list_size(app_data->timer_groups); ++i) {
    struct List* timer_group = list_get(app_data->timer_groups, i);
    list_apply(timer_group, (List_apply_fp_t)timer_destroy);
  }
  list_apply(app_data->timer_groups, (List_apply_fp_t)list_destroy);
  list_destroy(app_data->timer_groups);
  app_data->timer_groups = NULL;
  settings_destroy(app_data->settings);
  app_data->settings = NULL;
  free(app_data);
}

struct List* app_data_get_timer_groups(struct App_data* app_data) {
	if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null app_data pointer");
    return NULL;
  }
  return app_data->timer_groups;
}

struct Settings* app_data_get_settings(struct App_data* app_data) {
	if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null app_data pointer");
    return NULL;
  }
  return app_data->settings;
}

struct List* app_data_get_timer_group(struct App_data* app_data, int timer_group_index) {
  if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null app_data pointer");
    return NULL;
  }
  if (timer_group_index == INVALID_INDEX) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer group index");
    return NULL;
  }
  return list_get(app_data->timer_groups, timer_group_index);
}

struct Timer* app_data_get_timer(struct App_data* app_data, int timer_group_index, int timer_index) { 
  if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null app_data pointer");
    return NULL;
  }
  if (timer_group_index == INVALID_INDEX) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer group index");
    return NULL;
  }
  if (timer_index == INVALID_INDEX) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer index");
    return NULL;
  }
  struct List* timer_group = app_data_get_timer_group(app_data, timer_group_index);
  return list_get(timer_group, timer_index);
}
