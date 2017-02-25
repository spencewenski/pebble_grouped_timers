#ifndef UTILITY_H
#define UTILITY_H

#include <pebble.h>

void* safe_alloc(int size);

// [min, max)
// return non-zero (true) if in range, zero (false) otherwise
int in_range(int value, int min, int max);

// Wrap the value to be in [min, max)
int wrap_value(int value, int min, int max);

// Get the minimum value
int min(int a, int b);

// Get the maximum value
int max(int a, int b);

// Exit the app
void exit_app();
void click_handler_exit_app(ClickRecognizerRef recognizer, void* context);
void menu_click_handler_exit_app(MenuLayer* menu_layer, MenuIndex* cell_index, void* data);

#endif /*UTILITY_H*/
