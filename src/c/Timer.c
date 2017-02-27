#include "Timer.h"
#include "Utility.h"
#include "assert.h"
#include "persist_util.h"

#include <pebble.h>
#include <stdlib.h>
#include <math.h>

#define DEFAULT_VALUE 0
#define MAX_HOURS 60
#define MAX_MINUTES 60
#define MAX_SECONDS 60

static int get_max_value(enum Timer_field timer_field);

struct Timer {
  int id;
  int hours;
  int minutes;
  int seconds;
  int start_time_seconds; // Time in seconds since the timer was last paused
  int elapsed_seconds;    // How much of the timer has elapsed
};

struct Timer* timer_create(int timer_id) {
  struct Timer* timer = safe_alloc(sizeof(struct Timer));
  timer->id = timer_id;
  timer->hours = DEFAULT_VALUE;
  timer->minutes = DEFAULT_VALUE;
  timer->seconds = DEFAULT_VALUE;
  timer_reset(timer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Timer created with id: %d", timer_id);
  return timer;
}

void timer_destroy(struct Timer* timer)
{
  free(timer);
}

struct Timer* timer_load()
{
  struct Timer* timer = safe_alloc(sizeof(struct Timer));
  persist_read_data(g_current_persist_key++, timer, sizeof(struct Timer));
  return timer;
}

void timer_save(const struct Timer* timer)
{
  assert(timer);
  persist_write_data(g_current_persist_key++, timer, sizeof(struct Timer));
}

int timer_get_id(const struct Timer* timer)
{
  assert(timer);
  return timer->id;
}

void timer_set_field(struct Timer* timer, const enum Timer_field timer_field, int value)
{
  assert(timer);
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

static int get_max_value(enum Timer_field timer_field)
{
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

int timer_get_field(const struct Timer* timer, const enum Timer_field timer_field)
{
  assert(timer);
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

void timer_increment_field(struct Timer* timer, const enum Timer_field timer_field, int amount)
{
  assert(timer);
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

void timer_set_all(struct Timer* timer, int hours, int minutes, int seconds)
{
  timer_set_field(timer, TIMER_FIELD_HOURS, hours);
  timer_set_field(timer, TIMER_FIELD_MINUTES, minutes);
  timer_set_field(timer, TIMER_FIELD_SECONDS, seconds);
}

int timer_get_length_seconds(const struct Timer* timer)
{
  assert(timer);
  return (timer->hours * SECONDS_PER_HOUR) + (timer->minutes * SECONDS_PER_MINUTE) + timer->seconds;
}

int timer_get_field_remaining(const struct Timer* timer, const enum Timer_field timer_field)
{
  assert(timer);
  int remaining_seconds = timer_get_remaining_seconds(timer);
  if (remaining_seconds == timer_get_length_seconds(timer)) {
    return timer_get_field(timer, timer_field);
  }
  switch (timer_field) {
    case TIMER_FIELD_HOURS:
      return remaining_seconds / SECONDS_PER_HOUR;
    case TIMER_FIELD_MINUTES:
      return (remaining_seconds % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
    case TIMER_FIELD_SECONDS:
      return (remaining_seconds % SECONDS_PER_MINUTE);
    case TIMER_FIELD_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid timer field: %d", timer_field);
      return 0;
  }
}

int timer_get_remaining_seconds(const struct Timer* timer)
{
  assert(timer);
  // If the timer isn't started
  if (!timer_is_running(timer) && !timer_is_paused(timer)) {
    return timer_get_length_seconds(timer);
  }
  int remaining = timer_get_length_seconds(timer) - timer->elapsed_seconds;
  return remaining > 0 ? remaining : 0;
}

void timer_update(struct Timer* timer)
{
  assert(timer);
  if (!timer_is_running(timer)) {
    // Timer not running; nothing to update
    return ;
  }
  int current_time = time(NULL);
  timer->elapsed_seconds += current_time - timer->start_time_seconds;
  timer->start_time_seconds = current_time;
}

int timer_is_running(const struct Timer* timer)
{
  assert(timer);
  return timer->start_time_seconds > 0 ? 1 : 0;
}

int timer_is_paused(const struct Timer* timer)
{
  assert(timer);
  return timer->start_time_seconds <= 0 && timer->elapsed_seconds > 0 ? 1 : 0;
}

int timer_is_elapsed(const struct Timer* timer)
{
  assert(timer);
  return timer->elapsed_seconds >= timer_get_length_seconds(timer) ? 1 : 0;
}

void timer_start(struct Timer* timer)
{
  assert(timer);
  if (timer->start_time_seconds > 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Timer already started");
    return;
  }
  timer->start_time_seconds = time(NULL);
}

void timer_pause(struct Timer* timer)
{
  assert(timer);
  if (timer->start_time_seconds <= 0) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Timer not started");
    return;
  }
  timer->elapsed_seconds += time(NULL) - timer->start_time_seconds;
  timer->start_time_seconds = DEFAULT_VALUE;
}

// Reset timer back to its original value
void timer_reset(struct Timer* timer)
{
  assert(timer);
  timer->start_time_seconds = DEFAULT_VALUE;
  timer->elapsed_seconds = DEFAULT_VALUE;
}
