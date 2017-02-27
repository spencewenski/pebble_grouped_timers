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

static void init_fake_data(struct App_data* app_data);

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
  // init_fake_data(app_data);
  main_window_push(app_data);
}

static void init_fake_data(struct App_data* app_data)
{
  struct List* timer_groups = app_data_get_timer_groups(app_data);

  // Timer group 1
  struct Timer_group* timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  struct Timer* timer = timer_create();
  timer_set_all(timer, 0, 15, 0);
  timer_group_add_timer(timer_group, timer);

  timer = timer_create();
  timer_set_all(timer, 0, 45, 0);
  timer_group_add_timer(timer_group, timer);

  struct Settings* settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_WAIT_FOR_USER);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_NUDGE);

  // Timer group 2
  timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  timer = timer_create();
  timer_set_all(timer, 0, 0, 20);
  timer_group_add_timer(timer_group, timer);

  settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_AUTO);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_NUDGE);

  // Timer group 3
  timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  timer = timer_create();
  timer_set_all(timer, 0, 0, 5);
  timer_group_add_timer(timer_group, timer);

  timer = timer_create();
  timer_set_all(timer, 0, 0, 8);
  timer_group_add_timer(timer_group, timer);

  settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_AUTO);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_CONTINUOUS);

  // Timer group 4
  timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  timer = timer_create();
  timer_set_all(timer, 0, 0, 10);
  timer_group_add_timer(timer_group, timer);

  settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_WAIT_FOR_USER);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_NUDGE);

  // Timer group 4
  timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  timer = timer_create();
  timer_set_all(timer, 0, 0, 12);
  timer_group_add_timer(timer_group, timer);

  settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_WAIT_FOR_USER);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_CONTINUOUS);
}

static void deinit(struct App_data* app_data)
{
  app_data_save(app_data);
  app_data_destroy(app_data);
  // persist_delete(PERSIST_VERSION_KEY);
}
