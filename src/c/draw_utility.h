#pragma once

#include <pebble.h>

void menu_cell_draw_header(GContext* ctx, const Layer* cell_layer, const char* text);
void menu_cell_draw_text_row(GContext* ctx, const Layer* cell_layer, const char* text);