#include "main_window.h"
#include "App_data.h"
#include "List.h"
#include "Utility.h"
#include "timer_group_window.h"
#include "settings_window.h"
#include "globals.h"
#include "draw_utility.h"

#include <pebble.h>

#define MAIN_MENU_NUM_SECTIONS 2
#define SETTINGS_NUM_ROWS 2

static Window* s_main_window;
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
static void menu_cell_draw_timer_group_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data);

void main_window_push(struct App_data* app_data) {
  s_main_window = window_create();
  
  if (!s_main_window) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null main window");
    return;
  }
  
  window_set_user_data(s_main_window, app_data);
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load_handler,
    .unload = window_unload_handler
  });
  
  window_stack_push(s_main_window, false);
}

/*
WindowHandlers
*/
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
  
  app_data_set_current_timer_group_index(app_data, INVALID_INDEX);
  app_data_set_current_timer_index(app_data, INVALID_INDEX);
}

static void window_unload_handler(Window* window) {
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
  
  window_destroy(s_main_window);
  s_main_window = NULL;
}

/*
MenuLayerCallbacks
*/
static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data) {
  return MAIN_MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  struct App_data* app_data = data;
  
  switch (section_index) {
    case 0:
      // Timer groups
      if (!app_data) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Null data pointer");
        return 0;
      }
      return list_size(app_data_get_timer_groups(app_data));
    case 1:
      // Settings
      return SETTINGS_NUM_ROWS;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", section_index);
      return 0;
  }
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
      menu_cell_draw_timer_group_row(ctx, cell_layer, cell_index->row, data);
      return;
    case 1:
      if (cell_index->row == 0) {
        // New timer group
        menu_cell_draw_text_row(ctx, cell_layer, "New Group");
        return;
      }
      if (cell_index->row == 1) {
        // Settings
        menu_cell_draw_text_row(ctx, cell_layer, "Settings");
        return;
      }    
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
}

static void menu_cell_draw_timer_group_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data) {
  struct App_data* app_data = data;
  struct List* timer_groups = app_data_get_timer_groups(app_data);
  if (!in_range(row_index, 0, list_size(timer_groups))) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid row index: %d", row_index);
    return;
  }
  struct List* timer_group = list_get(timer_groups, row_index);

  if (!timer_group) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer_group pointer");
    return;
  }
  char menu_text[MENU_TEXT_LENGTH];
  char* timer_text = list_size(timer_group) == 1 ? "Timer" : "Timers";
  snprintf(menu_text, sizeof(menu_text), "%d %s", list_size(timer_group), timer_text);
  
  menu_cell_draw_text_row(ctx, cell_layer, menu_text);
}

static void menu_draw_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data) {
  switch (section_index) {
    case 0:
      menu_cell_draw_header(ctx, cell_layer, "Timer groups");
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
      // Timer group
      if (!in_range(cell_index->row, 0, list_size(app_data_get_timer_groups(app_data)))) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid row index: %d", cell_index->row);
        return;
      }
      app_data_set_current_timer_group_index(app_data, cell_index->row);
      timer_group_window_push(app_data);
      break;
    case 1:
      // Settings
      if (cell_index->row == 0) {
        // New timer group
        struct List* timer_groups = app_data_get_timer_groups(app_data);
        list_add(timer_groups, list_create());
        app_data_set_current_timer_group_index(app_data, list_size(timer_groups) - 1);
        timer_group_window_push(app_data);
      } else if (cell_index->row == 1) {
        // Settings
        settings_window_push(app_data);
      }
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
  // Refresh window
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
  menu_layer_reload_data(s_menu_layer);
}
