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
  int current_timer_group_index;   // index of the group currently being edited/viewed
  int current_timer_index;         // index of the timer currently being edited/viewed
};

struct App_data* app_data_create() {
	struct App_data* app_data = safe_alloc(sizeof(struct App_data));
  app_data->timer_groups = list_create();
  app_data->settings = settings_create();
  app_data->current_timer_group_index = INVALID_INDEX;
  app_data->current_timer_index = INVALID_INDEX;
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

void app_data_set_current_timer_group_index(struct App_data* app_data, int index) {
  if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null app_data pointer");
    return;
  }
  if (!in_range(index, 0, list_size(app_data->timer_groups))) {
    return;
  }
  app_data->current_timer_group_index = index;
}

void app_data_set_current_timer_index(struct App_data* app_data, int index) {
  if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null app_data pointer");
    return;
  }
  if (!in_range(index, 0, list_size(app_data->timer_groups))) {
    return;
  }
  app_data->current_timer_index = index;
}

struct List* app_data_get_current_timer_group(struct App_data* app_data) {
  if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null app_data pointer");
    return NULL;
  }
  if (app_data->current_timer_group_index == INVALID_INDEX) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer group index");
    return NULL;
  }
  return list_get(app_data->timer_groups, app_data->current_timer_group_index);
}

struct Timer* app_data_get_current_timer(struct App_data* app_data) {
  if (!app_data) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null app_data pointer");
    return NULL;
  }
  if (app_data->current_timer_group_index == INVALID_INDEX) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer group index");
    return NULL;
  }
  if (app_data->current_timer_index == INVALID_INDEX) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Invalid timer index");
    return NULL;
  }
  struct List* timer_group = app_data_get_current_timer_group(app_data);
  return list_get(timer_group, app_data->current_timer_index);
}
