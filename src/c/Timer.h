#pragma once

#define NUM_TIMER_FIELDS 3

struct Timer;

struct Timer* timer_create();
void timer_destroy(struct Timer* timer);

void timer_set_hours(struct Timer* timer, int hours);
void timer_set_minutes(struct Timer* timer, int minutes);
void timer_set_seconds(struct Timer* timer, int seconds);
void timer_set_all(struct Timer* timer, int hours, int minutes, int seconds);

void timer_increment_hours(struct Timer* timer, int amount);
void timer_increment_minutes(struct Timer* timer, int amount);
void timer_increment_seconds(struct Timer* timer, int amount);

int timer_get_hours(struct Timer* timer);
int timer_get_minutes(struct Timer* timer);
int timer_get_seconds(struct Timer* timer);