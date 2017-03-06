#include "main_window.h"
#include "App_data.h"
#include "List.h"
#include "Utility.h"
#include "timer_group_window.h"
#include "timer_countdown_window.h"
#include "settings_window.h"
#include "globals.h"
#include "draw_utility.h"
#include "Timer_group.h"
#include "assert.h"
#include "Timer.h"
#include "Settings.h"

#include <pebble.h>

#define MAIN_MENU_NUM_SECTIONS 2
#define SUBTITLE_TEXT_LENGTH 100

#define SETTINGS_NUM_ROWS SETTINGS_NUM_ROWS_IMPL
#ifdef NDEBUG
#define SETTINGS_NUM_ROWS_IMPL 2
#else
#define SETTINGS_NUM_ROWS_IMPL 3
#endif /* NDEBUG */

static Window* s_main_window;
static MenuLayer* s_menu_layer;
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
static void menu_cell_draw_timer_group_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data);
static void get_subtitle_text(char* buf, int buf_size, const struct Timer_group* timer_group);
#ifndef NDEBUG
static void create_test_data();
#endif /* NDEBUG */

void main_window_push()
{
  s_main_window = window_create();

  assert(s_main_window);

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load_handler,
    .appear = window_appear_handler,
    .unload = window_unload_handler
  });

  window_stack_push(s_main_window, false);
}

/*
WindowHandlers
*/
static void window_load_handler(Window* window)
{
  Layer* window_layer = window_get_root_layer(window);

  // Status bar layer
  s_status_bar_layer = status_bar_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar_layer));

  // Menu layer
  GRect bounds = layer_get_bounds(window_layer);
  bounds = status_bar_adjust_window_bounds(bounds);
  s_menu_layer = menu_layer_create(bounds);
  assert(s_menu_layer);

  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
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

  window_destroy(s_main_window);
  s_main_window = NULL;
}

/*
MenuLayerCallbacks
*/
static uint16_t menu_get_num_sections_callback(MenuLayer* menu_layer, void* data)
{
  return MAIN_MENU_NUM_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data)
{
  assert(menu_layer);

  switch (section_index) {
    case 0:
      // Timer groups
      return list_size(app_data_get_timer_groups(app_data_get()));
    case 1:
      // Settings
      return SETTINGS_NUM_ROWS;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", section_index);
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer* menu_layer, uint16_t section_index, void* data)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data)
{
  switch (cell_index->section) {
    case 0:
      menu_cell_draw_timer_group_row(ctx, cell_layer, cell_index->row, data);
      return;
    case 1:
      if (cell_index->row == 0) {
        // New timer group
        menu_cell_basic_draw(ctx, cell_layer, "New Group", NULL, NULL);
        return;
      }
      if (cell_index->row == 1) {
        // Settings
        menu_cell_basic_draw(ctx, cell_layer, "Settings", NULL, NULL);
        return;
      }
#ifndef NDEBUG
      if (cell_index->row == 2) {
        // Create test data
        menu_cell_basic_draw(ctx, cell_layer, "Create test data", NULL, NULL);
        return;
      }
#endif /* NDEBUG */
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
}

static void menu_cell_draw_timer_group_row(GContext* ctx, const Layer* cell_layer, uint16_t row_index, void* data)
{
  assert(ctx);
  assert(cell_layer);

  struct List* timer_groups = app_data_get_timer_groups(app_data_get());
  assert(in_range(row_index, 0, list_size(timer_groups)));

  struct Timer_group* timer_group = list_get(timer_groups, row_index);
  assert(timer_group);

  char menu_text[MENU_TEXT_LENGTH];
  int num_timers = timer_group_size(timer_group);
  char* timer_text = num_timers == 1 ? "Timer" : "Timers";
  snprintf(menu_text, sizeof(menu_text), "%d %s", num_timers, timer_text);

  char subtitle_text[SUBTITLE_TEXT_LENGTH];
  subtitle_text[0] = '\0';
  get_subtitle_text(subtitle_text, SUBTITLE_TEXT_LENGTH, timer_group);
  menu_cell_basic_draw(ctx, cell_layer, menu_text,
    strlen(subtitle_text) > 0 ? subtitle_text : NULL, NULL);
}

static void get_subtitle_text(char* buf, int buf_size, const struct Timer_group* timer_group)
{
  assert(buf);
  assert(timer_group);
  int end_index = 0;
  char timer_text[TIMER_TEXT_LENGTH];

  for (int i = 0; i < timer_group_size(timer_group) && end_index < buf_size; ++i) {
    struct Timer* timer = timer_group_get_timer(timer_group, i);
    get_timer_text(timer_text, sizeof(timer_text),
        timer_get_field(timer, TIMER_FIELD_HOURS),
        timer_get_field(timer, TIMER_FIELD_MINUTES),
        timer_get_field(timer, TIMER_FIELD_SECONDS));
    end_index += snprintf(buf + end_index, buf_size - end_index, timer_text);
    if (i < timer_group_size(timer_group) - 1) {
      end_index += snprintf(buf + end_index, buf_size - end_index, ",  ");
    }
  }
}


static void menu_draw_header_callback(GContext* ctx, const Layer* cell_layer, uint16_t section_index, void* data)
{
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

static void menu_select_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data)
{
  switch (cell_index->section) {
    case 0:
      // Timer group
      assert(in_range(cell_index->row, 0, list_size(app_data_get_timer_groups(app_data_get()))));
      timer_group_window_push(cell_index->row);
      break;
    case 1:
      if (cell_index->row == 0) {
        // New timer group
        struct List* timer_groups = app_data_get_timer_groups(app_data_get());
        list_add(timer_groups, timer_group_create());
        timer_group_window_push(list_size(timer_groups) - 1);
      } else if (cell_index->row == 1) {
        // Settings
        settings_window_push(INVALID_INDEX);
      }
#ifndef NDEBUG
      else if (cell_index->row == 2) {
        create_test_data();
      }
#endif /* NDEBUG */
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid section index: %d", cell_index->section);
      return;
  }
  // Refresh window
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
  menu_layer_reload_data(s_menu_layer);
}

static void menu_select_long_click_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data)
{
  switch (cell_index->section) {
    case 0:
      // Start first timer in group
      assert(in_range(cell_index->row, 0, list_size(app_data_get_timer_groups(app_data_get()))));
      if (timer_group_size(app_data_get_timer_group(app_data_get(), cell_index->row)) <= 0) {
        break;
      }
      timer_start(app_data_get_timer(app_data_get(), cell_index->row, 0));
      timer_countdown_window_push(cell_index->row, 0);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Only support long click on timer groups");
      return;
  }
  // Refresh window
  // Todo: determine if these lines are necessary
  layer_mark_dirty(menu_layer_get_layer(s_menu_layer));
  menu_layer_reload_data(s_menu_layer);
}

#ifndef NDEBUG
static void create_test_data()
{
  struct App_data* app_data = app_data_get();
  struct List* timer_groups = app_data_get_timer_groups(app_data);

  // Timer group 1
  struct Timer_group* timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  struct Timer* timer = timer_create(app_data_get_next_timer_id(app_data));
  timer_set_all(timer, 0, 45, 0);
  timer_group_add_timer(timer_group, timer);

  timer = timer_create(app_data_get_next_timer_id(app_data));
  timer_set_all(timer, 0, 15, 0);
  timer_group_add_timer(timer_group, timer);

  struct Settings* settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_WAIT_FOR_USER);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_NUDGE);

  // Timer group 2
  timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  timer = timer_create(app_data_get_next_timer_id(app_data));
  timer_set_all(timer, 0, 0, 5);
  timer_group_add_timer(timer_group, timer);

  timer = timer_create(app_data_get_next_timer_id(app_data));
  timer_set_all(timer, 0, 0, 10);
  timer_group_add_timer(timer_group, timer);

  settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_AUTO);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_NUDGE);

  // Timer group 3
  timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  timer = timer_create(app_data_get_next_timer_id(app_data));
  timer_set_all(timer, 0, 0, 20);
  timer_group_add_timer(timer_group, timer);

  settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_AUTO);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_NUDGE);

  // Timer group 4
  timer_group = timer_group_create();
  list_add(timer_groups, timer_group);

  timer = timer_create(app_data_get_next_timer_id(app_data));
  timer_set_all(timer, 0, 0, 5);
  timer_group_add_timer(timer_group, timer);

  timer = timer_create(app_data_get_next_timer_id(app_data));
  timer_set_all(timer, 0, 1, 0);
  timer_group_add_timer(timer_group, timer);

  settings = timer_group_get_settings(timer_group);
  settings_set_repeat_style(settings, REPEAT_STYLE_GROUP);
  settings_set_progress_style(settings, PROGRESS_STYLE_AUTO);
  settings_set_vibrate_style(settings, VIBRATE_STYLE_NUDGE);
}
#endif /* NDEBUG */
