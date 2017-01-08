#pragma once

struct App_data;

struct App_data* app_data_create();

void app_data_destroy(struct App_data* app_data);

struct List* app_data_get_timer_groups(struct App_data* app_data);

struct Settings* app_data_get_settings(struct App_data* app_data);