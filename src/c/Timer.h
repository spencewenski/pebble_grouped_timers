#pragma once

#define NUM_TIMER_FIELDS 3

struct Timer;

enum Timer_field {
  TIMER_FIELD_HOURS,
  TIMER_FIELD_MINUTES,
  TIMER_FIELD_SECONDS,
  TIMER_FIELD_INVALID
};

struct Timer* timer_create();
void timer_destroy(struct Timer* timer);

void timer_set_field(struct Timer* timer, const enum Timer_field timer_field, int value);
int timer_get_field(struct Timer* timer, const enum Timer_field timer_field);
void timer_increment_field(struct Timer* timer, const enum Timer_field timer_field, int amount);
void timer_set_all(struct Timer* timer, int hours, int minutes, int seconds);