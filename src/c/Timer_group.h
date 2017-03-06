#ifndef TIMER_GROUP_H
#define TIMER_GROUP_H

struct Timer_group;
struct Timer;
struct List;
struct Settings;

struct Timer_group* timer_group_create();
void timer_group_destroy(struct Timer_group* timer_group);

struct Timer_group* timer_group_load();
void timer_group_save(const struct Timer_group* timer_group);

// Settings
struct Settings* timer_group_get_settings(const struct Timer_group* timer_group);

// Timers
struct List* timer_group_get_timers(const struct Timer_group* timer_group);
void timer_group_add_timer(struct Timer_group* timer_group, struct Timer* timer);
void timer_group_remove_timer(struct Timer_group* timer_group, int index);
int timer_group_size(const struct Timer_group* timer_group);
struct Timer* timer_group_get_timer(const struct Timer_group* timer_group, int index);
// Return the timer with the given ID. Return NULL if no timer in this group has the given ID.
struct Timer* timer_group_get_timer_by_id(const struct Timer_group* timer_group, int timer_id);
// Return the index of the timer with the given ID. Return negative if no timer
// has the given ID.
int timer_group_get_timer_index(const struct Timer_group* timer_group, int timer_id);
void timer_group_cancel_wakeups(const struct Timer_group* timer_group);

#endif /*TIMER_GROUP_H*/
