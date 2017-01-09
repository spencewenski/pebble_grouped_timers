#include "timer_group_window.h"
#include "App_data.h"
#include "globals.h"

#include <pebble.h>

#define MAIN_MENU_NUM_SECTIONS 2
#define SETTINGS_NUM_ROWS 1

static Window* timer_group_window_s;
static MenuLayer* menu_layer_s;

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

void timer_group_window_push(struct App_data* app_data, int timer_group_index) {
  timer_group_window_s = window_create();
  
  if (!timer_group_window_s) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null timer group window");
  }
  
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
}

static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data) {
  return 0;
}

static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  return 0;
}

static int16_t menu_get_cell_height_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data) {
  return 0;
}

static int16_t menu_get_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  return 0;
}

static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data) {

}

static void menu_draw_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data) {

}

static void menu_select_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data) {

}

