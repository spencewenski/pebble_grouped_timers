#include "timer_group_window.h"
#include "App_data.h"
#include "List.h"
#include "Utility.h"
#include "Timer.h"
#include "globals.h"
#include "timer_edit_window.h"
#include "draw_utility.h"

#include <pebble.h>

#define MENU_NUM_SECTIONS 2
#define SETTINGS_NUM_ROWS 1

static Window* s_timer_group_window;
static MenuLayer* s_menu_layer;

// WindowHandlers
static void window_load_handler(Window* window);
static void window_unload_handler(Window* window);

// MenuLayerCallbacks
static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data);
static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data);
static int16_t menu_get_cell_height_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data);
static int16_t menu_get_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* data);
static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data);
static void menu_draw_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data);
static void menu_select_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data);

// Helpers
static void menu_cell_draw_timer_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data);

void timer_group_window_push(struct App_data* app_data) {
  s_timer_group_window = window_create();
  
  if (!s_timer_group_window) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer group window");
    return;
  }
    
  window_set_user_data(s_timer_group_window, app_data);
  
  window_set_window_handlers(s_timer_group_window, (WindowHandlers) {
    .load = window_load_handler,
    .unload = window_unload_handler
  });
  
  window_stack_push(s_timer_group_window, false);
}

// WindowHandlers
static void window_load_handler(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_menu_layer = menu_layer_create(bounds);
  if (!s_menu_layer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null menu layer");
  }
  struct App_data* app_data = window_get_user_data(window);
  
  menu_layer_set_callbacks(s_menu_layer, app_data, (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_click_callback
  });
  
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload_handler(Window* window) {
  struct App_data* app_data = window_get_user_data(window);
  app_data_set_current_timer_group_index(app_data, INVALID_INDEX);
  
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
  
  window_destroy(s_timer_group_window);  
  s_timer_group_window = NULL;
}

static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data) {
  return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  struct App_data* app_data = data;
  switch (section_index) {
    case 0:
      // Timers
      return list_size(app_data_get_current_timer_group(app_data));
    case 1:
      // Settings
      return SETTINGS_NUM_ROWS;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", section_index);
      return 0;
  }
  return 0;
}

static int16_t menu_get_cell_height_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data) {
  return MENU_CELL_HEIGHT;
}

static int16_t menu_get_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data) {
  switch (cell_index->section) {
    case 0:
      menu_cell_draw_timer_row(ctx, cell_layer, cell_index->row, data);
      return;
    case 1:
      // New timer
      menu_cell_draw_text_row(ctx, cell_layer, "New Timer");
      return;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
}

static void menu_cell_draw_timer_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data) {
  struct App_data* app_data = data;
  struct List* timer_group = app_data_get_current_timer_group(app_data);
  if (!timer_group) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer_group pointer");
    return;
  }
  if (!in_range(row_index, 0, list_size(timer_group))) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid row index: %d", row_index);
    return;
  }
  struct Timer* timer = list_get(timer_group, row_index);
  char menu_text[MENU_TEXT_LENGTH];
  if (timer_get_field(timer, TIMER_FIELD_HOURS) > 0) {
    snprintf(menu_text, sizeof(menu_text), "%d:%.2d:%.2d",
             timer_get_field(timer, TIMER_FIELD_HOURS),
             timer_get_field(timer, TIMER_FIELD_MINUTES),
             timer_get_field(timer, TIMER_FIELD_SECONDS));
  } else {
    snprintf(menu_text, sizeof(menu_text), "%d:%.2d",
             timer_get_field(timer, TIMER_FIELD_MINUTES),
             timer_get_field(timer, TIMER_FIELD_SECONDS));
  }
  
  menu_cell_draw_text_row(ctx, cell_layer, menu_text);
}

static void menu_draw_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data) {
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

static void menu_select_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data) {
  struct App_data* app_data = data;
  
  switch (cell_index->section) {
    case 0:
      // Edit/create timer
      app_data_set_current_timer_index(app_data, cell_index->row);
      timer_edit_window_push(app_data);
      break;
    case 1:
      // Edit/create timer
      list_add(app_data_get_current_timer_group(app_data), timer_create());
      app_data_set_current_timer_index(app_data, list_size(app_data_get_current_timer_group(app_data)) - 1);
      timer_edit_window_push(app_data);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
  // Refresh window
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
  menu_layer_reload_data(s_menu_layer);
}

