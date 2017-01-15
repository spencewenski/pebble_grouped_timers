#include "timer_edit_window.h"
#include "App_data.h"
#include "globals.h"
#include "Timer.h"
#include "List.h"

#include <pebble.h>

#define CLICK_REPEAT_INTERVAL_MS 100

static Window* s_timer_edit_window;
static TextLayer* s_timer_text_layer;
static int s_edit_timer_field_num;
static int s_timer_group_index;
static int s_timer_index;

static char s_timer_text_buffer[TIMER_TEXT_LENGTH];

// WindowHandlers
static void window_load_handler(Window* window);
static void window_unload_handler(Window* window);

// Click handlers
static void click_config_provider(void* context);
static void click_handler_up(ClickRecognizerRef recognizer, void* context);
static void click_handler_select(ClickRecognizerRef recognizer, void* context);
static void click_handler_down(ClickRecognizerRef recognizer, void* context);
static void click_handler_back(ClickRecognizerRef recognizer, void* context);

// Timer display
static void update_timer_text_layer(struct Timer* timer);

// Helpers
static enum Timer_field get_timer_field(int timer_edit_index);

void timer_edit_window_push(struct App_data* app_data, int timer_group_index, int timer_index) {
  s_timer_edit_window = window_create();
  
  if (!s_timer_edit_window) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null main window");
    return;
  }
  
  s_timer_group_index = timer_group_index;
  s_timer_index = timer_index;
  
  window_set_user_data(s_timer_edit_window, app_data);
  
  window_set_window_handlers(s_timer_edit_window, (WindowHandlers) {
    .load = window_load_handler,
    .unload = window_unload_handler
  });
  
  window_stack_push(s_timer_edit_window, false);
}

static void window_load_handler(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  window_set_click_config_provider(window, click_config_provider);

  GRect window_bounds = layer_get_bounds(window_layer);
  
  // Setup other static variables
  s_edit_timer_field_num = 0;
  s_timer_text_buffer[0] = '\0';
  
  // Setup timer text layer
  GRect timer_bounds = window_bounds;
  timer_bounds.size.h = TIMER_TEXT_HEIGHT;
  grect_align(&timer_bounds, &window_bounds, GAlignLeft, false);
  s_timer_text_layer = text_layer_create(timer_bounds);
  if (!s_timer_text_layer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null menu layer");
    return;
  }
  text_layer_set_text_color(s_timer_text_layer, GColorBlack);
  text_layer_set_background_color(s_timer_text_layer, GColorWhite);
  text_layer_set_text_alignment(s_timer_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_timer_text_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(s_timer_text_layer));
  // Set the timer text
  struct App_data* app_data = window_get_user_data(window);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  update_timer_text_layer(timer);  
}

static void window_unload_handler(Window* window) {
  struct App_data* app_data = window_get_user_data(window);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  if (timer_get_length_seconds(timer) <= 0) {
    list_remove(app_data_get_timer_group(app_data, s_timer_group_index), s_timer_index);
  }
  
  text_layer_destroy(s_timer_text_layer);
  s_timer_text_layer = NULL;

  window_destroy(s_timer_edit_window);
  s_timer_edit_window = NULL;  
}

// Click handlers
static void click_config_provider(void* context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, CLICK_REPEAT_INTERVAL_MS, click_handler_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler_select);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, CLICK_REPEAT_INTERVAL_MS, click_handler_down);
  window_single_click_subscribe(BUTTON_ID_BACK, click_handler_back);
}

static void click_handler_up(ClickRecognizerRef recognizer, void* context) {
  struct App_data* app_data = window_get_user_data(context);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  timer_increment_field(timer, get_timer_field(s_edit_timer_field_num), 1);
  update_timer_text_layer(timer);
}

static void click_handler_select(ClickRecognizerRef recognizer, void* context) {
  ++s_edit_timer_field_num;
  if (s_edit_timer_field_num > 2) {
    window_stack_pop(false);
  }
}

static void click_handler_down(ClickRecognizerRef recognizer, void* context) {
  struct App_data* app_data = window_get_user_data(context);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  timer_increment_field(timer, get_timer_field(s_edit_timer_field_num), -1);
  update_timer_text_layer(timer);
}

static void click_handler_back(ClickRecognizerRef recognizer, void* context) {
  --s_edit_timer_field_num;
  if (s_edit_timer_field_num < 0) {
    window_stack_pop(false);
  }
}

static void update_timer_text_layer(struct Timer* timer) {
  snprintf(s_timer_text_buffer, sizeof(s_timer_text_buffer), "%.2d:%.2d:%.2d",
          timer_get_field(timer, TIMER_FIELD_HOURS),
          timer_get_field(timer, TIMER_FIELD_MINUTES),
          timer_get_field(timer, TIMER_FIELD_SECONDS));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Timer value - %s", s_timer_text_buffer);
  text_layer_set_text(s_timer_text_layer, s_timer_text_buffer);
  layer_mark_dirty(text_layer_get_layer(s_timer_text_layer));
}

static enum Timer_field get_timer_field(int timer_edit_index) {
  switch (timer_edit_index) {
    case 0:
      return TIMER_FIELD_HOURS;
    case 1:
      return TIMER_FIELD_MINUTES;
    case 2:
      return TIMER_FIELD_SECONDS;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid timer index: %d", timer_edit_index);
      return TIMER_FIELD_INVALID;
  }
}