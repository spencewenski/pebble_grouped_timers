#include "main_window.h"
#include "App_data.h"
#include "List.h"
#include "Utility.h"
#include "timer_group_window.h"
#include "settings_window.h"
#include "dimensions.h"

#include <pebble.h>

#define MAIN_MENU_NUM_SECTIONS 2
#define SETTINGS_NUM_ROWS 2

static Window* main_window_s;
static MenuLayer* menu_layer_s;
static StatusBarLayer* status_bar_s;

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
static void menu_cell_draw_header(GContext* ctx, const Layer* cell_layer, const char* text);
static void menu_cell_draw_timer_group_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data);
static void menu_cell_draw_text_row(GContext* ctx, const Layer* cell_layer, const char* text);

void main_window_push(struct App_data* app_data) {
  main_window_s = window_create();
  
  if (!main_window_s) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null main window");
    return;
  }
  
  window_set_user_data(main_window_s, app_data);
  
  window_set_window_handlers(main_window_s, (WindowHandlers) {
    .load = window_load_handler,
    .unload = window_unload_handler
  });
  
  window_stack_push(main_window_s, false);
}

/*
WindowHandlers
*/
static void window_load_handler(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  menu_layer_s = menu_layer_create(bounds);
  if (!menu_layer_s) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null menu layer");
  }
  struct App_data* app_data = window_get_user_data(window);
  
  menu_layer_set_callbacks(menu_layer_s, app_data, (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_click_callback
  });
  
  menu_layer_set_click_config_onto_window(menu_layer_s, window);
  
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer_s));
  
  // Create the StatusBarLayer
//   status_bar_s = status_bar_layer_create();
//   status_bar_layer_set_separator_mode(status_bar_s, StatusBarLayerSeparatorModeDotted);
//   layer_add_child(window_layer, status_bar_layer_get_layer(status_bar_s));
}

static void window_unload_handler(Window* window) {
  menu_layer_destroy(menu_layer_s);
  window_destroy(main_window_s);
  main_window_s = NULL;
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

static void menu_cell_draw_text_row(GContext* ctx, const Layer* cell_layer, const char* text) {
  GRect bounds = layer_get_bounds(cell_layer);
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  
  graphics_draw_text(ctx,
                     text,
                     font,
                     bounds,
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft,
                     NULL);
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

static void menu_cell_draw_header(GContext* ctx, const Layer* cell_layer, const char* text) {
  GSize size = layer_get_frame(cell_layer).size;
  graphics_draw_text(ctx,
                     text,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(0,0,size.w,size.h),
                     GTextOverflowModeTrailingEllipsis,
                     GTextAlignmentLeft,
                     NULL);
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
      timer_group_window_push(app_data, cell_index->row);
      return;
    case 1:
      // Settings
      if (cell_index->row == 0) {
        // New timer group
        struct List* timer_groups = app_data_get_timer_groups(app_data);
        list_add(timer_groups, list_create());
        timer_group_window_push(app_data, list_size(timer_groups) - 1);
      } else if (cell_index->row == 1) {
        // Settings
        settings_window_push(app_data);
      }
      // Refresh window
      layer_mark_dirty(menu_layer_get_layer(menu_layer_s));
      menu_layer_reload_data(menu_layer_s);
      return;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
}
