#include "timer_edit_window.h"
#include "App_data.h"
#include "globals.h"
#include "Timer.h"
#include "List.h"
#include "Timer_group.h"
#include "assert.h"
#include "draw_utility.h"
#include "Wakeup_manager.h"
#include "wakeup_util.h"

#include <pebble.h>

#define CLICK_REPEAT_INTERVAL_MS 100

static Window* s_timer_edit_window;
static TextLayer* s_timer_text_layer;
static int s_edit_timer_field_num;
static int s_timer_group_index;
static int s_timer_index;
static StatusBarLayer* s_status_bar_layer;

static char s_timer_text_buffer[TIMER_TEXT_LENGTH];

// WindowHandlers
static void window_load_handler(Window* window);
static void window_unload_handler(Window* window);
static void window_appear_handler(Window* window);

// Click handlers
static void click_config_provider(void* context);
static void click_handler_up(ClickRecognizerRef recognizer, void* context);
static void click_handler_select(ClickRecognizerRef recognizer, void* context);
static void click_handler_down(ClickRecognizerRef recognizer, void* context);
static void click_handler_back(ClickRecognizerRef recognizer, void* context);

// Timer display
static void update_timer_text_layer(const struct Timer* timer);

// Helpers
static enum Timer_field get_timer_field(int timer_edit_index);

void timer_edit_window_push(int timer_group_index, int timer_index)
{
  s_timer_edit_window = window_create();

  assert(s_timer_edit_window);

  s_timer_group_index = timer_group_index;
  s_timer_index = timer_index;

  window_set_user_data(s_timer_edit_window, app_data_get());

  window_set_window_handlers(s_timer_edit_window, (WindowHandlers) {
    .load = window_load_handler,
      .appear = window_appear_handler,
    .unload = window_unload_handler
  });

  window_stack_push(s_timer_edit_window, false);
}

static void window_load_handler(Window* window)
{
  Layer* window_layer = window_get_root_layer(window);
  window_set_click_config_provider(window, click_config_provider);

  // Setup other static variables
  s_edit_timer_field_num = 0;
  s_timer_text_buffer[0] = '\0';

  // Status bar layer
  s_status_bar_layer = status_bar_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar_layer));

  // Setup timer text layer
  GRect window_bounds = layer_get_bounds(window_layer);
  window_bounds = status_bar_adjust_window_bounds(window_bounds);
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
  struct Timer* timer = app_data_get_timer(app_data_get(), s_timer_group_index, s_timer_index);
  update_timer_text_layer(timer);
}

static void window_appear_handler(Window* window)
{
  timer_cancel_wakeup(app_data_get_timer(app_data_get(), s_timer_group_index, s_timer_index));
}

static void window_unload_handler(Window* window)
{
  struct App_data* app_data = app_data_get();
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  if (timer_get_length_seconds(timer) <= 0) {
    timer_group_remove_timer(app_data_get_timer_group(app_data, s_timer_group_index), s_timer_index);
    timer_destroy(timer);
    timer = NULL;
  }

  status_bar_layer_destroy(s_status_bar_layer);
  s_status_bar_layer = NULL;

  text_layer_destroy(s_timer_text_layer);
  s_timer_text_layer = NULL;

  window_destroy(s_timer_edit_window);
  s_timer_edit_window = NULL;
}

// Click handlers
static void click_config_provider(void* context)
{
  window_single_repeating_click_subscribe(BUTTON_ID_UP, CLICK_REPEAT_INTERVAL_MS, click_handler_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler_select);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, CLICK_REPEAT_INTERVAL_MS, click_handler_down);
  window_single_click_subscribe(BUTTON_ID_BACK, click_handler_back);
}

static void click_handler_up(ClickRecognizerRef recognizer, void* context)
{
  struct Timer* timer = app_data_get_timer(app_data_get(), s_timer_group_index, s_timer_index);
  timer_increment_field(timer, get_timer_field(s_edit_timer_field_num), 1);
  update_timer_text_layer(timer);
}

static void click_handler_select(ClickRecognizerRef recognizer, void* context)
{
  ++s_edit_timer_field_num;
  if (s_edit_timer_field_num > 2) {
    window_stack_pop(false);
  }
}

static void click_handler_down(ClickRecognizerRef recognizer, void* context)
{
  struct Timer* timer = app_data_get_timer(app_data_get(), s_timer_group_index, s_timer_index);
  timer_increment_field(timer, get_timer_field(s_edit_timer_field_num), -1);
  update_timer_text_layer(timer);
}

static void click_handler_back(ClickRecognizerRef recognizer, void* context)
{
  --s_edit_timer_field_num;
  if (s_edit_timer_field_num < 0) {
    struct Timer* timer = app_data_get_timer(app_data_get(), s_timer_group_index, s_timer_index);
    // Set all the fields to zero so the timer is deleted when the window exits
    timer_set_field(timer, TIMER_FIELD_HOURS, 0);
    timer_set_field(timer, TIMER_FIELD_MINUTES, 0);
    timer_set_field(timer, TIMER_FIELD_SECONDS, 0);
    window_stack_pop(false);
  }
}

static void update_timer_text_layer(const struct Timer* timer)
{
  snprintf(s_timer_text_buffer, sizeof(s_timer_text_buffer), "%.2d:%.2d:%.2d",
          timer_get_field(timer, TIMER_FIELD_HOURS),
          timer_get_field(timer, TIMER_FIELD_MINUTES),
          timer_get_field(timer, TIMER_FIELD_SECONDS));
  text_layer_set_text(s_timer_text_layer, s_timer_text_buffer);
  layer_mark_dirty(text_layer_get_layer(s_timer_text_layer));
}

static enum Timer_field get_timer_field(int timer_edit_index)
{
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
