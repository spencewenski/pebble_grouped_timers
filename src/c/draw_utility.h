#ifndef DRAW_UTILITY_H
#define DRAW_UTILITY_H

#include <pebble.h>

void menu_cell_draw_header(GContext* ctx, const Layer* cell_layer, const char* text);

int16_t menu_cell_get_height_round(MenuLayer* menu_layer, MenuIndex* cell_index, void* data);

void get_timer_text(char* buf, int buf_size, int hours, int minutes, int seconds);

StatusBarLayer* status_bar_create();
GRect status_bar_adjust_window_bounds(GRect bounds);

#endif /*DRAW_UTILITY_H*/
