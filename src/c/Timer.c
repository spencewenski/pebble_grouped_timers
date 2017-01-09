#include "Timer.h"
#include "Utility.h"

#include <pebble.h>

#define MAX_HOURS 60
#define MAX_MINUTES 60
#define MAX_SECONDS 60

struct Timer {
  int hours;
  int minutes;
  int seconds;
};

struct Timer* timer_create() {
  return safe_alloc(sizeof(struct Timer));
}

void timer_destroy(struct Timer* timer) {
  free(timer);
}

void timer_set_hours(struct Timer* timer, int hours) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "set_hours: NULL Timer");
    return;
  }
  if (!in_range(hours, 0, MAX_HOURS)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "set_hours: %d not in range", hours);
    return;
  }
  timer->hours = hours;
}

void timer_set_minutes(struct Timer* timer, int minutes) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "set_minutes: NULL Timer");
    return;
  }
  if (!in_range(minutes, 0, MAX_MINUTES)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "set_minutes: %d not in range", minutes);
    return;
  }
  timer->minutes = minutes;
}

void timer_set_seconds(struct Timer* timer, int seconds) {
  if (!timer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "set_seconds: NULL Timer");
    return;
  }
  if (!in_range(seconds, 0, MAX_SECONDS)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "set_seconds: %d not in range", seconds);
    return;
  }
  timer->seconds = seconds;
}

void timer_set_all(struct Timer* timer, int hours, int minutes, int seconds) {
  timer_set_hours(timer, hours);
  timer_set_minutes(timer, minutes);
  timer_set_seconds(timer, seconds);
}
