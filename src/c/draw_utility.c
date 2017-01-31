#include "draw_utility.h"
#include "assert.h"

#include <pebble.h>

static void menu_cell_draw_header_centered(GContext* ctx, const Layer* cell_layer, const char* text);

void menu_cell_draw_header(GContext* ctx, const Layer* cell_layer, const char* text)
{
  assert(ctx);
  assert(cell_layer);
  assert(text);

  PBL_IF_ROUND_ELSE(
    menu_cell_draw_header_centered(ctx, cell_layer, text),
    menu_cell_basic_header_draw(ctx, cell_layer, text)
    );
}

static void menu_cell_draw_header_centered(GContext* ctx, const Layer* cell_layer, const char* text)
{
  GSize size = layer_get_frame(cell_layer).size;
  graphics_draw_text(ctx,
                     text,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(0,0,size.w,size.h),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter,
                     NULL);
}


void get_timer_text(char* buf, int buf_size, int hours, int minutes,
  int seconds)
{
  assert(buf);
  if (hours > 0) {
    snprintf(buf, buf_size, "%d:%.2d:%.2d", hours, minutes, seconds);
  } else if (minutes > 0) {
    snprintf(buf, buf_size, "%d:%.2d", minutes, seconds);
  } else {
    snprintf(buf, buf_size, ":%.2d", seconds);
  }
}
