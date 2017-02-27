#ifndef APP_DATA_H
#define APP_DATA_H

struct App_data;
struct List;
struct Timer;
struct Timer_group;

void app_data_destroy(struct App_data* app_data);

struct App_data* app_data_load();
void app_data_save(const struct App_data* app_data);

struct List* app_data_get_timer_groups(const struct App_data* app_data);

struct Settings* app_data_get_settings(const struct App_data* app_data);

struct Timer_group* app_data_get_timer_group(const struct App_data* app_data, int timer_group_index);
struct Timer* app_data_get_timer(const struct App_data* app_data, int timer_group_index, int timer_index);
// Return the timer with the given ID. Return null if no timer has the given ID.
struct Timer* app_data_get_timer_by_id(const struct App_data* app_data, int timer_id);

// Get the next available timer id
int app_data_get_next_timer_id(const struct App_data* app_data);

#endif /*APP_DATA_H*/
