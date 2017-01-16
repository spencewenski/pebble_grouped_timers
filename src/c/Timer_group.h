#pragma once

struct Timer_group;
struct List;

struct Timer_group* timer_group_create();
void timer_group_destroy(struct Timer_group* timer_group);

struct List* timer_group_get_timers(struct Timer_group* timer_group);