#pragma once

struct App_data;

/*
Push the settings window. Use the same window for app and timer group settings.
@param timer_group: Index of the timer group, or INVALID_INDEX if editing app
                    settings.
*/
void settings_window_push(struct App_data* app_data, int timer_group);