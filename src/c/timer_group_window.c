#include "timer_group_window.h"
#include "App_data.h"
#include "dimensions.h"
#include "List.h"
#include "Utility.h"
#include "Timer.h"

#include <pebble.h>

#define MENU_NUM_SECTIONS 2
#define SETTINGS_NUM_ROWS 1
#define INVALID_INDEX -1

static Window* timer_group_window_s;
static MenuLayer* menu_layer_s;
static int current_timer_group_index_s = INVALID_INDEX;

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
static void menu_cell_draw_text_row(GContext* ctx, const Layer* cell_layer, const char* text);
static void menu_cell_draw_timer_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data);

void timer_group_window_push(struct App_data* app_data, int timer_group_index) {
  timer_group_window_s = window_create();
  
  if (!timer_group_window_s) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer group window");
    return;
  }
  
  current_timer_group_index_s = timer_group_index;
  
  window_set_user_data(timer_group_window_s, app_data);
  
  window_set_window_handlers(timer_group_window_s, (WindowHandlers) {
    .load = window_load_handler,
    .unload = window_unload_handler
  });
  
  window_stack_push(timer_group_window_s, false);
}

// WindowHandlers
static void window_load_handler(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  menu_layer_s = menu_layer_create(bounds);
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
}

static void window_unload_handler(Window* window) {
  menu_layer_destroy(menu_layer_s);
  window_destroy(timer_group_window_s);
  timer_group_window_s = NULL;
  current_timer_group_index_s = INVALID_INDEX;
}

static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data) {
  return MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  struct App_data* app_data = data;
  switch (section_index) {
    case 0:
      // Timers
      return list_size(list_get(app_data_get_timer_groups(app_data), current_timer_group_index_s));
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
  struct List* timer_group = list_get(app_data_get_timer_groups(app_data), current_timer_group_index_s);
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
  if (timer_get_hours(timer) > 0) {
    snprintf(menu_text, sizeof(menu_text), "%d:%.2d:%.2d",
             timer_get_hours(timer),
             timer_get_minutes(timer),
             timer_get_seconds(timer));
  } else {
    snprintf(menu_text, sizeof(menu_text), "%d:%.2d",
             timer_get_minutes(timer),
             timer_get_seconds(timer));
  }
  
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
  struct List* timer_groups = app_data_get_timer_groups(app_data);
  struct Timer* new_timer = timer_create();
  timer_set_all(new_timer, 0, 0, 25);
  
  switch (cell_index->section) {
    case 0:
      // Edit/create timer
      break;
    case 1:
      // Edit/create timer
//       timer_create_window_push(app_data, current_timer_group_index_s);
      list_add(list_get(timer_groups, current_timer_group_index_s), new_timer);
      // Refresh window
      layer_mark_dirty(menu_layer_get_layer(menu_layer_s));
      menu_layer_reload_data(menu_layer_s);
      return;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
}

