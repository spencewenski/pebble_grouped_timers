#include <pebble.h>
#include "draw_utility.h"

void menu_cell_draw_header(GContext* ctx, const Layer* cell_layer, const char* text) {
  GSize size = layer_get_frame(cell_layer).size;
  graphics_draw_text(ctx,
                     text,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(0,0,size.w,size.h),
                     GTextOverflowModeTrailingEllipsis,
                     PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft),
                     NULL);
}

void menu_cell_draw_text_row(GContext* ctx, const Layer* cell_layer, const char* text) {
  GRect bounds = layer_get_bounds(cell_layer);
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  
  graphics_draw_text(ctx,
                     text,
                     font,
                     bounds,
                     GTextOverflowModeTrailingEllipsis,
                     PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft),
                     NULL);
}
