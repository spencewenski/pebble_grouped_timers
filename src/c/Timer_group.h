#pragma once

struct Timer_group;
struct Timer;
struct List;

struct Timer_group* timer_group_create();
void timer_group_destroy(struct Timer_group* timer_group);

struct List* timer_group_get_timers(struct Timer_group* timer_group);

void timer_group_add_timer(struct Timer_group* timer_group, struct Timer* timer);
void timer_group_remove_timer(struct Timer_group* timer_group, int index);
int timer_group_size(struct Timer_group* timer_group);
struct Timer* timer_group_get_timer(struct Timer_group* timer_group, int index);
