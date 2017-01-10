#pragma once

struct App_data;
struct List;
struct Timer;

struct App_data* app_data_create();

void app_data_destroy(struct App_data* app_data);

struct List* app_data_get_timer_groups(struct App_data* app_data);

struct Settings* app_data_get_settings(struct App_data* app_data);

void app_data_set_current_timer_group_index(struct App_data* app_data, int index);
void app_data_set_current_timer_index(struct App_data* app_data, int index);
struct List* app_data_get_current_timer_group(struct App_data* app_data);
struct Timer* app_data_get_current_timer(struct App_data* app_data);
