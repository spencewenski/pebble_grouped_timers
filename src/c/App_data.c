#include "App_data.h"
#include "Utility.h"
#include "List.h"
#include "Settings.h"
#include "Timer.h"
#include "globals.h"
#include "assert.h"
#include "Timer_group.h"
#include "persist_util.h"
#include "Wakeup_manager.h"

#include <pebble.h>


static struct App_data* app_data_create();
static struct App_data* app_data_load();
static void app_data_save(const struct App_data* app_data);
static void app_data_destroy_intern(struct App_data* app_data);

struct App_data {
  struct List* timer_groups; // List of Lists
  struct Settings* settings;
  struct Wakeup_manager* wakeup_manager;
};
static struct App_data* s_app_data = NULL;

struct App_data* app_data_get()
{
  if (s_app_data) {
    return s_app_data;
  }
  s_app_data = app_data_load();
  return s_app_data;
}

void app_data_destroy() {
  if (!s_app_data) {
    return;
  }
  app_data_save(s_app_data);
  app_data_destroy_intern(s_app_data);
  s_app_data = NULL;
}

static void app_data_save(const struct App_data* app_data)
{
  assert(app_data);
  persist_init_save();
  list_save(app_data->timer_groups, (List_for_each_fp_t) timer_group_save);
  settings_save(app_data->settings);
  wakeup_manager_save(app_data->wakeup_manager);
  persist_finish_save();
}

static void app_data_destroy_intern(struct App_data* app_data)
{
  assert(app_data);
  list_for_each(app_data->timer_groups, (List_for_each_fp_t)timer_group_destroy);
  list_destroy(app_data->timer_groups);
  app_data->timer_groups = NULL;
  settings_destroy(app_data->settings);
  app_data->settings = NULL;
  wakeup_manager_destroy(app_data->wakeup_manager);
  app_data->wakeup_manager = NULL;
  free(app_data);
}

struct App_data* app_data_load()
{
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
  app_data->wakeup_manager = wakeup_manager_load();
  persist_finish_load();
  return app_data;
}

static struct App_data* app_data_create()
{
  struct App_data* app_data = safe_alloc(sizeof(struct App_data));
  app_data->timer_groups = list_create();
  app_data->settings = settings_create();
  app_data->wakeup_manager = wakeup_manager_create();
  return app_data;
}

struct List* app_data_get_timer_groups(const struct App_data* app_data)
{
  assert(app_data);
  return app_data->timer_groups;
}

struct Settings* app_data_get_settings(const struct App_data* app_data)
{
  assert(app_data);
  return app_data->settings;
}

struct Timer_group* app_data_get_timer_group(const struct App_data* app_data, int timer_group_index)
{
  assert(app_data);
  return list_get(app_data->timer_groups, timer_group_index);
}

struct Timer* app_data_get_timer(const struct App_data* app_data, int timer_group_index, int timer_index)
{
  assert(app_data);
  struct Timer_group* timer_group = app_data_get_timer_group(app_data, timer_group_index);
  assert(timer_group);
  return timer_group_get_timer(timer_group, timer_index);
}

struct Timer* app_data_get_timer_by_id(const struct App_data* app_data, int timer_id)
{
  assert(app_data);
  assert(timer_id >= 0);
  for (int i = 0; i < list_size(app_data->timer_groups); ++i) {
    struct Timer* timer = timer_group_get_timer_by_id(list_get(app_data->timer_groups, i), timer_id);
    if (timer) {
      return timer;
    }
  }
  return NULL;
}

int app_data_get_timer_group_index_by_timer_id(const struct App_data* app_data, int timer_id)
{
  assert(app_data);
  assert(timer_id >= 0);
  for (int i = 0; i < list_size(app_data->timer_groups); ++i) {
    if (timer_group_get_timer_by_id(list_get(app_data->timer_groups, i), timer_id)) {
      return i;
    }
  }
  return -1;
}

int app_data_get_next_timer_id(const struct App_data* app_data)
{
  assert(app_data);
  int timer_id = 0;
  while (app_data_get_timer_by_id(app_data, timer_id)) {
    timer_id++;
  }
  return timer_id;
}

struct Wakeup_manager* app_data_get_wakeup_manager(const struct App_data* app_data)
{
  assert(app_data);
  return app_data->wakeup_manager;
}
