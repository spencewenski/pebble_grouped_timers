#include "Timer.h"
#include "Utility.h"

#include <pebble.h>
#include <stdlib.h>

#define DEFAULT_VALUE 0
#define MAX_HOURS 60
#define MAX_MINUTES 60
#define MAX_SECONDS 60

static int get_max_value(enum Timer_field timer_field);

struct Timer {
  int hours;
  int minutes;
  int seconds;
};

struct Timer* timer_create() {
  struct Timer* timer = safe_alloc(sizeof(struct Timer));
  timer->hours = DEFAULT_VALUE;
  timer->minutes = DEFAULT_VALUE;
  timer->seconds = DEFAULT_VALUE;
  return timer;
}

void timer_destroy(struct Timer* timer) {
  free(timer);
}

void timer_set_field(struct Timer* timer, const enum Timer_field timer_field, int value) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer pointer");
    return;
  }
  switch (timer_field) {
    case TIMER_FIELD_HOURS:
      timer->hours = wrap_value(value, 0, get_max_value(timer_field));
      return;
    case TIMER_FIELD_MINUTES:
      timer->minutes = wrap_value(value, 0, get_max_value(timer_field));
      return;
    case TIMER_FIELD_SECONDS:
      timer->seconds = wrap_value(value, 0, get_max_value(timer_field));
      return;
    case TIMER_FIELD_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid timer field: %d", timer_field);
      return;
  }
}

static int get_max_value(enum Timer_field timer_field) {
  switch (timer_field) {
    case TIMER_FIELD_HOURS:
      return MAX_HOURS;
    case TIMER_FIELD_MINUTES:
      return MAX_MINUTES;
    case TIMER_FIELD_SECONDS:
      return MAX_SECONDS;
    case TIMER_FIELD_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid timer field: %d", timer_field);
      return 0;
  }
}

int timer_get_field(struct Timer* timer, const enum Timer_field timer_field) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer pointer");
    return 0;
  }
  switch (timer_field) {
    case TIMER_FIELD_HOURS:
      return timer->hours;
    case TIMER_FIELD_MINUTES:
      return timer->minutes;
    case TIMER_FIELD_SECONDS:
      return timer->seconds;
    case TIMER_FIELD_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid timer field: %d", timer_field);
      return 0;
  }
}

int timer_get_length_seconds(struct Timer* timer) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer pointer");
    return 0;
  }
  return (timer->hours * SECONDS_PER_HOUR) + (timer->minutes * SECONDS_PER_MINUTE) + timer->seconds;
}

void timer_increment_field(struct Timer* timer, const enum Timer_field timer_field, int amount) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer pointer");
    return;
  }
  switch (timer_field) {
    case TIMER_FIELD_HOURS:
      timer_set_field(timer, timer_field, timer->hours + amount);
      return;
    case TIMER_FIELD_MINUTES:
      timer_set_field(timer, timer_field, timer->minutes + amount);
      return;
    case TIMER_FIELD_SECONDS:
      timer_set_field(timer, timer_field, timer->seconds + amount);
      return;
    case TIMER_FIELD_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid timer field: %d", timer_field);
      return;
  }
}

void timer_set_all(struct Timer* timer, int hours, int minutes, int seconds) {
  timer_set_field(timer, TIMER_FIELD_HOURS, hours);
  timer_set_field(timer, TIMER_FIELD_MINUTES, minutes);
  timer_set_field(timer, TIMER_FIELD_SECONDS, seconds);
}