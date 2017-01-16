#include "Timer_group.h"
#include "List.h"
#include "utility.h"
#include "Timer.h"

#include <pebble.h>

struct Timer_group {
  struct List* timers;
};

struct Timer_group* timer_group_create() {
  struct Timer_group* timer_group = safe_alloc(sizeof(struct Timer_group));
  if (!timer_group) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer group pointer");
    return NULL;
  }
  timer_group->timers = list_create();
  return timer_group;
}

void timer_group_destroy(struct Timer_group* timer_group) {
  if (!timer_group) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer group pointer");
    return;
  }
  list_apply(timer_group->timers, (List_apply_fp_t)timer_destroy);
  list_destroy(timer_group->timers);
  free(timer_group);
}

struct List* timer_group_get_timers(struct Timer_group* timer_group) {
  if (!timer_group) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer group pointer");
    return NULL;
  }
  return timer_group->timers;
}
