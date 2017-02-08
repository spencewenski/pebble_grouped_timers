#include "settings_window.h"
#include "Settings.h"
#include "globals.h"
#include "App_data.h"
#include "assert.h"
#include "Timer_group.h"
#include "draw_utility.h"

#include <pebble.h>

static Window* s_settings_window;
static MenuLayer* s_menu_layer;
static int s_timer_group_index;

// WindowHandlers
static void window_load_handler(Window* window);
static void window_unload_handler(Window* window);

// MenuLayerCallbacks
static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data);
static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data);
static int16_t menu_get_cell_height_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data);
static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data);
static void menu_select_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data);

// Helpers
static struct Settings* get_settings(const struct App_data* app_data, int timer_group_index);
static enum Settings_field get_settings_field(int settings_field_index);
static enum Repeat_style get_next_repeat_style(enum Repeat_style repeat_style);
static enum Progress_style get_next_progress_style(enum Progress_style progress_style);
static enum Vibrate_style get_next_vibrate_style(enum Vibrate_style vibrate_style);

void settings_window_push(struct App_data* app_data, int timer_group)
{
  s_settings_window = window_create();
  
  assert(s_settings_window);

  s_timer_group_index = timer_group;
  
  window_set_user_data(s_settings_window, app_data);
  
  window_set_window_handlers(s_settings_window, (WindowHandlers) {
    .load = window_load_handler,
    .unload = window_unload_handler
  });
  
  window_stack_push(s_settings_window, false);
}

// WindowHandlers
static void window_load_handler(Window* window)
{
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_menu_layer = menu_layer_create(bounds);
  assert(s_menu_layer);

  struct App_data* app_data = window_get_user_data(window);
  
  menu_layer_set_callbacks(s_menu_layer, app_data, (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_click_callback
  });
  
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload_handler(Window* window)
{
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
  
  window_destroy(s_settings_window);
  s_settings_window = NULL;
}

// MenuLayerCallbacks
static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data)
{
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data)
{
  return NUM_SETTINGS_FIELDS;
}

static int16_t menu_get_cell_height_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data)
{
  return menu_cell_get_height(menu_layer, cell_index);
}

static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data)
{
  struct App_data* app_data = data;
  struct Settings* settings = get_settings(app_data, s_timer_group_index);
  
  GSize size = layer_get_bounds(cell_layer).size;
  GFont big_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GFont small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  enum Settings_field settings_field = get_settings_field(cell_index->row);

  const char* small_text;
  switch (settings_field) {
    case SETTINGS_FIELD_REPEAT_STYLE:
      small_text = settings_get_repeat_style_text(settings_get_repeat_style(settings));
      break;
    case SETTINGS_FIELD_PROGRESS_STYLE:
      small_text = settings_get_progress_style_text(settings_get_progress_style(settings));
      break;
    case SETTINGS_FIELD_VIBRATE_STYLE:
      small_text = settings_get_vibrate_style_text(settings_get_vibrate_style(settings));
      break;
    case SETTINGS_FIELD_INVALID: // intentional fall through
    default:
      small_text = "";
      break;
  }

  menu_cell_basic_draw(ctx, cell_layer,
    settings_get_settings_field_text(settings_field), small_text, NULL);
}

static struct Settings* get_settings(const struct App_data* app_data, int timer_group_index)
{
  if (s_timer_group_index < 0) {
    return app_data_get_settings(app_data);
  } else {
    return timer_group_get_settings(app_data_get_timer_group(app_data, s_timer_group_index));
  }
}

static enum Settings_field get_settings_field(int settings_field_index)
{
  switch (settings_field_index) {
    case 0:
      return SETTINGS_FIELD_REPEAT_STYLE;
    case 1:
      return SETTINGS_FIELD_PROGRESS_STYLE;
    case 2:
      return SETTINGS_FIELD_VIBRATE_STYLE;
    default:
      return SETTINGS_FIELD_INVALID;
  }
}

static void menu_select_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data)
{
  struct App_data* app_data = data;
  struct Settings* settings = get_settings(app_data, s_timer_group_index);
  enum Settings_field settings_field = get_settings_field(cell_index->row);
  
  switch (settings_field) {
    case SETTINGS_FIELD_REPEAT_STYLE:
      settings_set_repeat_style(settings, get_next_repeat_style(settings_get_repeat_style(settings)));
      break;
    case SETTINGS_FIELD_PROGRESS_STYLE:
      settings_set_progress_style(settings, get_next_progress_style(settings_get_progress_style(settings)));
      break;
    case SETTINGS_FIELD_VIBRATE_STYLE:
      settings_set_vibrate_style(settings, get_next_vibrate_style(settings_get_vibrate_style(settings)));
      break;
    case SETTINGS_FIELD_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid settings field: %d", settings_field);
      break;
  }
  menu_layer_reload_data(s_menu_layer);
}

static enum Repeat_style get_next_repeat_style(enum Repeat_style repeat_style)
{
  switch (repeat_style) {
    case REPEAT_STYLE_NONE:
      return REPEAT_STYLE_SINGLE;
    case REPEAT_STYLE_SINGLE:
      return REPEAT_STYLE_GROUP;
    case REPEAT_STYLE_GROUP:
      return REPEAT_STYLE_NONE;
    case REPEAT_STYLE_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid repeat style: %d", repeat_style);
      return REPEAT_STYLE_INVALID;
  }
}

static enum Progress_style get_next_progress_style(enum Progress_style progress_style)
{
  switch (progress_style) {
    case PROGRESS_STYLE_NONE:
      return PROGRESS_STYLE_AUTO;
    case PROGRESS_STYLE_AUTO:
      return PROGRESS_STYLE_WAIT_FOR_USER;
    case PROGRESS_STYLE_WAIT_FOR_USER:
      return PROGRESS_STYLE_NONE;
    case PROGRESS_STYLE_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid progress style: %d", progress_style);
      return PROGRESS_STYLE_INVALID;
  }
}

static enum Vibrate_style get_next_vibrate_style(enum Vibrate_style vibrate_style)
{
  switch (vibrate_style) {
    case VIBRATE_STYLE_NONE:
      return VIBRATE_STYLE_NUDGE;
    case VIBRATE_STYLE_NUDGE:
      return VIBRATE_STYLE_CONTINUOUS;
    case VIBRATE_STYLE_CONTINUOUS:
      return VIBRATE_STYLE_NONE;
    case VIBRATE_STYLE_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid vibrate style: %d", vibrate_style);
      return VIBRATE_STYLE_INVALID;
  }
}
