#include "Timer_group.h"
#include "List.h"
#include "utility.h"
#include "Timer.h"
#include "assert.h"
#include "Settings.h"

#include <pebble.h>

struct Timer_group {
  struct List* timers;
  struct Settings* settings;
};

struct Timer_group* timer_group_create()
{
  struct Timer_group* timer_group = safe_alloc(sizeof(struct Timer_group));
  timer_group->timers = list_create();
  timer_group->settings = settings_create();
  return timer_group;
}

void timer_group_destroy(struct Timer_group* timer_group)
{
  assert(timer_group);
  list_apply(timer_group->timers, (List_apply_fp_t)timer_destroy);
  list_destroy(timer_group->timers);
  settings_destroy(timer_group->settings);
  free(timer_group);
}

struct Timer_group* timer_group_load()
{
  struct Timer_group* timer_group = safe_alloc(sizeof(struct Timer_group));
  timer_group->timers = list_load((List_load_item_fp_t) timer_load);
  timer_group->settings = settings_load();
  return timer_group;
}

void timer_group_save(const struct Timer_group* timer_group)
{
  assert(timer_group);
  list_save(timer_group->timers, (List_apply_fp_t) timer_save);
  settings_save(timer_group->settings);
}

struct Settings* timer_group_get_settings(const struct Timer_group* timer_group)
{
  assert(timer_group);

  return timer_group->settings;
}

struct List* timer_group_get_timers(const struct Timer_group* timer_group)
{
  if (!timer_group) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer group pointer");
    return NULL;
  }
  return timer_group->timers;
}

void timer_group_add_timer(struct Timer_group* timer_group, struct Timer* timer)
{
  assert(timer_group);
  assert(timer);

  list_add(timer_group->timers, timer);
}

void timer_group_remove_timer(struct Timer_group* timer_group, int index)
{
  assert(timer_group);

  list_remove(timer_group->timers, index);
}

int timer_group_size(const struct Timer_group* timer_group)
{
  assert(timer_group);

  return list_size(timer_group->timers);
}

struct Timer* timer_group_get_timer(const struct Timer_group* timer_group, int index)
{
  assert(timer_group);

  return list_get(timer_group->timers, index);
}
