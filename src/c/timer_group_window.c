#include "timer_group_window.h"
#include "App_data.h"
#include "List.h"
#include "Utility.h"
#include "Timer.h"
#include "globals.h"
#include "timer_edit_window.h"
#include "timer_countdown_window.h"
#include "draw_utility.h"
#include "Timer_group.h"
#include "assert.h"
#include "settings_window.h"

#include <pebble.h>

#define MENU_NUM_SECTIONS 2
#define SETTINGS_NUM_ROWS 3

static Window* s_timer_group_window;
static MenuLayer* s_menu_layer;
static int s_timer_group_index;
static StatusBarLayer* s_status_bar_layer;

// WindowHandlers
static void window_load_handler(Window* window);
static void window_appear_handler(Window* window);
static void window_unload_handler(Window* window);

// MenuLayerCallbacks
static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data);
static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data);
static int16_t menu_get_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* data);
static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data);
static void menu_draw_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data);
static void menu_select_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data);
static void menu_select_long_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data);

// Helpers
static void menu_cell_draw_timer_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data);

void timer_group_window_push(int timer_group)
{
  s_timer_group_window = window_create();

  assert(s_timer_group_window);

  s_timer_group_index = timer_group;

  window_set_window_handlers(s_timer_group_window, (WindowHandlers) {
    .load = window_load_handler,
    .appear = window_appear_handler,
    .unload = window_unload_handler
  });

  window_stack_push(s_timer_group_window, false);
}

// WindowHandlers
static void window_load_handler(Window* window)
{
  Layer* window_layer = window_get_root_layer(window);

  // Status bar layer
  s_status_bar_layer = status_bar_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar_layer));

  // Timer group window
  GRect bounds = layer_get_bounds(window_layer);
  bounds = status_bar_adjust_window_bounds(bounds);
  s_menu_layer = menu_layer_create(bounds);
  assert(s_menu_layer);

  menu_layer_set_callbacks(s_menu_layer, app_data_get(), (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_cell_height = PBL_IF_ROUND_ELSE(menu_cell_get_height_round, NULL),
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_click_callback,
    .select_long_click = menu_select_long_click_callback
  });

  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_appear_handler(Window* window)
{
  menu_layer_reload_data(s_menu_layer);
}

static void window_unload_handler(Window* window)
{
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;

  status_bar_layer_destroy(s_status_bar_layer);
  s_status_bar_layer = NULL;

  window_destroy(s_timer_group_window);
  s_timer_group_window = NULL;
}

static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data)
{
  return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data)
{
  switch (section_index) {
    case 0:
      // Timers
      return timer_group_size(app_data_get_timer_group(app_data_get(), s_timer_group_index));
    case 1:
      // Settings
      return SETTINGS_NUM_ROWS;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", section_index);
      return 0;
  }
  return 0;
}

static int16_t menu_get_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data)
{
  switch (cell_index->section) {
    case 0:
      menu_cell_draw_timer_row(ctx, cell_layer, cell_index->row, data);
      return;
    case 1:
      if (cell_index->row == 0) {
        // New timer
        menu_cell_basic_draw(ctx, cell_layer, "New Timer", NULL, NULL);
        return;
      }
      if (cell_index->row == 1) {
        // Settings
        menu_cell_basic_draw(ctx, cell_layer, "Settings", NULL, NULL);
        return;
      }
      if (cell_index->row == 2) {
        // Settings
        menu_cell_basic_draw(ctx, cell_layer, "Delete Group", NULL, NULL);
        return;
      }
      return;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
}

static void menu_cell_draw_timer_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data)
{
  assert(ctx);
  assert(cell_layer);
  assert(data);

  struct Timer_group* timer_group = app_data_get_timer_group(app_data_get(), s_timer_group_index);
  assert(timer_group);
  struct Timer* timer = timer_group_get_timer(timer_group, row_index);
  assert(timer);

  char menu_text[MENU_TEXT_LENGTH];
  get_timer_text(menu_text, sizeof(menu_text),
        timer_get_field(timer, TIMER_FIELD_HOURS),
        timer_get_field(timer, TIMER_FIELD_MINUTES),
        timer_get_field(timer, TIMER_FIELD_SECONDS));
  menu_cell_basic_draw(ctx, cell_layer, menu_text, NULL, NULL);
}

static void menu_draw_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data)
{
  switch (section_index) {
    case 0:
      menu_cell_draw_header(ctx, cell_layer, "Timers");
      return;
    case 1:
      // Settings
      menu_cell_draw_header(ctx, cell_layer, "Settings");
      return;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", section_index);
      return;
  }
}

static void menu_select_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data)
{
  struct App_data* app_data = app_data_get();

  switch (cell_index->section) {
    case 0:
      // Edit/create timer
      timer_countdown_window_push(s_timer_group_index, cell_index->row);
      break;
    case 1:
      if (cell_index->row == 0) {
        // Edit/create timer
        timer_group_add_timer(app_data_get_timer_group(app_data, s_timer_group_index), timer_create(app_data_get_next_timer_id(app_data)));
        timer_edit_window_push(s_timer_group_index, timer_group_size(app_data_get_timer_group(app_data, s_timer_group_index)) - 1);
      } else if (cell_index->row == 1) {
        // Settings
        settings_window_push(s_timer_group_index);
      } else if (cell_index->row == 2) {
        // Delete group
        struct Timer_group* timer_group = app_data_get_timer_group(app_data, s_timer_group_index);
        timer_group_cancel_wakeups(timer_group);
        list_remove(app_data_get_timer_groups(app_data), s_timer_group_index);
        timer_group_destroy(timer_group);
        timer_group = NULL;
        window_stack_pop(false);
        return;
      }
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
  // Refresh window
  // Todo: determine if these lines are necessary
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
  menu_layer_reload_data(s_menu_layer);
}

static void menu_select_long_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data)
{
  switch (cell_index->section) {
    case 0:
      // Edit timer
      timer_edit_window_push(s_timer_group_index, cell_index->row);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Only support long click on timers");
      return;
  }
  // Refresh window
  // Todo: determine if these lines are necessary
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
  menu_layer_reload_data(s_menu_layer);
}
