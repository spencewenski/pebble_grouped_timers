#include "draw_utility.h"
#include "assert.h"
#include "globals.h"

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
                     GRect(0, 0, size.w, size.h),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentCenter,
                     NULL);
}

int16_t menu_cell_get_height_round(MenuLayer* menu_layer, MenuIndex* cell_index, void* data)
{
  if (cell_index->section == menu_layer_get_selected_index(menu_layer).section &&
      cell_index->row == menu_layer_get_selected_index(menu_layer).row) {
    return MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT;
  }
  return MENU_CELL_ROUND_UNFOCUSED_SHORT_CELL_HEIGHT;
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

StatusBarLayer* status_bar_create()
{
  StatusBarLayer* status_bar_layer = status_bar_layer_create();
  return status_bar_layer;
}

GRect status_bar_adjust_window_bounds(GRect bounds)
{
  bounds.origin.y += STATUS_BAR_LAYER_HEIGHT;
  bounds.size.h -= STATUS_BAR_LAYER_HEIGHT;
  return bounds;
}
