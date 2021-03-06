#ifndef TIMER_H
#define TIMER_H

#define NUM_TIMER_FIELDS 3

struct Timer;

enum Timer_field {
  TIMER_FIELD_HOURS,
  TIMER_FIELD_MINUTES,
  TIMER_FIELD_SECONDS,
  TIMER_FIELD_INVALID
};

// Timer id should be retrieved from app_data_get_next_timer_id
struct Timer* timer_create(int timer_id);
void timer_destroy(struct Timer* timer);

struct Timer* timer_load();
void timer_save(const struct Timer* timer);

int timer_get_id(const struct Timer* timer);

void timer_set_field(struct Timer* timer, const enum Timer_field timer_field, int value);
int timer_get_field(const struct Timer* timer, const enum Timer_field timer_field);
void timer_increment_field(struct Timer* timer, const enum Timer_field timer_field, int amount);
void timer_set_all(struct Timer* timer, int hours, int minutes, int seconds);

int timer_get_length_seconds(const struct Timer* timer);
int timer_get_field_remaining(const struct Timer* timer, const enum Timer_field timer_field);
int timer_get_remaining_seconds(const struct Timer* timer);
// Return non-zero if timer is running, zero otherwise
// Client should call timer_update to get the most accurate result
int timer_is_running(const struct Timer* timer);
// Return non-zero if timer is paused, zero otherwise
// Client should call timer_update to get the most accurate result
int timer_is_paused(const struct Timer* timer);
// Return non-zero if timer is elapsed, zero otherwise
// Client should call timer_update to get the most accurate result
int timer_is_elapsed(const struct Timer* timer);

// Start/resume the timer
void timer_start(struct Timer* timer);
// Pause the timer
void timer_pause(struct Timer* timer);
// Reset timer back to its original value
void timer_reset(struct Timer* timer);
// Update the timer
void timer_update(struct Timer* timer);

#endif /*TIMER_H*/
