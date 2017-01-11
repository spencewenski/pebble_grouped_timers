// Some code borrowed from https://github.com/YclepticStudios/pebble-timer-plus/blob/master/src/timer.c

#include "Timer.h"
#include "Utility.h"

#include <pebble.h>

#define DEFAULT_VALUE 0
#define MS_IN_SECOND 1000
#define MS_IN_MINUTE 60000
#define MS_IN_HOUR 3600000
#define SECONDS_IN_MINUTE 60
#define MINUTES_IN_HOUR 60

struct Timer {
  int64_t length_ms;
};

struct Timer* timer_create() {
  struct Timer* timer = safe_alloc(sizeof(struct Timer));
  timer->length_ms = DEFAULT_VALUE;
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
  timer_increment_field(timer, timer_field, -timer_get_field(timer, timer_field));
  timer_increment_field(timer, timer_field, value);
}

int timer_get_field(struct Timer* timer, const enum Timer_field timer_field) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer pointer");
    return 0;
  }
  switch (timer_field) {
    case TIMER_FIELD_HOURS:
      return timer->length_ms / MS_IN_HOUR;
    case TIMER_FIELD_MINUTES:
      return (timer->length_ms % MS_IN_HOUR) / MS_IN_MINUTE;
    case TIMER_FIELD_SECONDS:
      return (timer->length_ms % MS_IN_MINUTE) / MS_IN_SECOND;
    case TIMER_FIELD_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid timer field: %d", timer_field);
      return 0;
  }
}

// todo: bounds checking
void timer_increment_field(struct Timer* timer, const enum Timer_field timer_field, int amount) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer pointer");
    return;
  }
  switch (timer_field) {
    case TIMER_FIELD_HOURS:
      timer->length_ms += (amount * MS_IN_HOUR);
      break;
    case TIMER_FIELD_MINUTES:
      timer->length_ms += (amount * MS_IN_MINUTE);
      break;
    case TIMER_FIELD_SECONDS:
      timer->length_ms += (amount * MS_IN_SECOND);
      break;
    case TIMER_FIELD_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid timer field: %d", timer_field);
      break;
  }
}

void timer_set_all(struct Timer* timer, int hours, int minutes, int seconds) {
  timer_set_field(timer, TIMER_FIELD_HOURS, hours);
  timer_set_field(timer, TIMER_FIELD_MINUTES, minutes);
  timer_set_field(timer, TIMER_FIELD_SECONDS, seconds);
}