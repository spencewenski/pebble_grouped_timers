#pragma once

struct Timer;

struct Timer* create_timer();
void destroy_timer(struct Timer* timer);
void set_hours(struct Timer* timer, int hours);
void set_minutes(struct Timer* timer, int minutes);
void set_seconds(struct Timer* timer, int seconds);
void set_all(struct Timer* timer, int hours, int minutes, int seconds);
