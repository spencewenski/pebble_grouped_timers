#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

/*
Push the settings window. Use the same window for app and timer group settings.
@param timer_group: Index of the timer group, or INVALID_INDEX if editing app
                    settings.
*/
void settings_window_push(int timer_group);

#endif /*SETTINGS_WINDOW_H*/
