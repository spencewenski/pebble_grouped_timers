#include "timer_countdown_window.h"
#include "timer_edit_window.h"
#include "globals.h"
#include "App_data.h"
#include "Timer.h"
#include "Settings.h"
#include "List.h"
#include "Timer_group.h"
#include "assert.h"
#include "draw_utility.h"
#include "Utility.h"

#include <pebble.h>

static Window* s_timer_countdown_window;
static TextLayer* s_timer_countdown_text_layer;
static TextLayer* s_timer_length_text_layer;
static int s_timer_group_index;
static int s_timer_index;
static AppTimer* s_app_timer_handle;
static AppTimer* s_app_timer_vibrate_handle;
static StatusBarLayer* s_status_bar_layer;

static char s_timer_countdown_text_buffer[TIMER_TEXT_LENGTH];
static char s_timer_length_text_buffer[TIMER_TEXT_LENGTH];

// Window Handlers
static void window_load_handler(Window* window);
static void window_appear_handler(Window* window);
static void window_unload_handler(Window* window);

// Timer display
static void update_timer_countdown_text_layer(struct Timer* timer);
static void update_timer_length_text_layer(struct Timer* timer);

// Click handlers
static void click_config_provider(void* context);
static void click_handler_up(ClickRecognizerRef recognizer, void* context);
static void click_handler_select(ClickRecognizerRef recognizer, void* context);
static void click_handler_down(ClickRecognizerRef recognizer, void* context);

// Timer handler
static void start_app_timer(int delay, AppTimerCallback app_timer_callback, void* data);
static void cancel_app_timers();
static void timer_handler(void* data);
static void vibrate_timer_handler(void* data);

// Helpers
static void update_current_timer(struct App_data* app_data);

void timer_countdown_window_push(struct App_data* app_data, int timer_group_index, int timer_index)
{
  s_timer_countdown_window = window_create();

  assert(s_timer_countdown_window);

  s_timer_group_index = timer_group_index;
  s_timer_index = timer_index;

  window_set_user_data(s_timer_countdown_window, app_data);

  window_set_window_handlers(s_timer_countdown_window, (WindowHandlers) {
    .load = window_load_handler,
    .appear = window_appear_handler,
    .unload = window_unload_handler
  });

  window_stack_push(s_timer_countdown_window, false);
}

static void window_load_handler(Window* window)
{
  Layer* window_layer = window_get_root_layer(window);

  window_set_click_config_provider(window, click_config_provider);

  // Setup other static variables
  s_timer_countdown_text_buffer[0] = '\0';
  s_timer_length_text_buffer[0] = '\0';

  struct App_data* app_data = window_get_user_data(window);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);

  // Status bar layer
  s_status_bar_layer = status_bar_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar_layer));

  // Countdown layer
  GRect window_bounds = layer_get_bounds(window_layer);
  window_bounds = status_bar_adjust_window_bounds(window_bounds);
  GRect timer_bounds = window_bounds;
  timer_bounds.size.h = TIMER_TEXT_HEIGHT;
  grect_align(&timer_bounds, &window_bounds, GAlignLeft, false);
  s_timer_countdown_text_layer = text_layer_create(timer_bounds);
  assert(s_timer_countdown_text_layer);
  text_layer_set_text_color(s_timer_countdown_text_layer, GColorBlack);
  text_layer_set_background_color(s_timer_countdown_text_layer, GColorWhite);
  text_layer_set_text_alignment(s_timer_countdown_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_timer_countdown_text_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(s_timer_countdown_text_layer));
  update_timer_countdown_text_layer(timer);

  // Setup timer length layer
  GRect timer_length_bounds = timer_bounds;
  timer_length_bounds.size.h = TIMER_TEXT_HEIGHT_SM;
  grect_align(&timer_length_bounds, &timer_bounds, GAlignBottom, false);
  s_timer_length_text_layer = text_layer_create(timer_length_bounds);
  assert(s_timer_length_text_layer);
  text_layer_set_text_color(s_timer_length_text_layer, GColorBlack);
  text_layer_set_background_color(s_timer_length_text_layer, GColorWhite);
  text_layer_set_text_alignment(s_timer_length_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_timer_length_text_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  layer_add_child(window_layer, text_layer_get_layer(s_timer_length_text_layer));
  update_timer_length_text_layer(timer);
}

static void window_appear_handler(Window* window)
{
  struct App_data* app_data = window_get_user_data(window);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  if (!timer) {
    window_stack_pop(false);
    return;
  }
  update_timer_countdown_text_layer(timer);
  update_timer_length_text_layer(timer);
  start_app_timer(0, timer_handler, app_data);
}

static void window_unload_handler(Window* window)
{
  cancel_app_timers();

  text_layer_destroy(s_timer_length_text_layer);
  s_timer_length_text_layer = NULL;

  text_layer_destroy(s_timer_countdown_text_layer);
  s_timer_countdown_text_layer = NULL;

  status_bar_layer_destroy(s_status_bar_layer);
  s_status_bar_layer = NULL;

  window_destroy(s_timer_countdown_window);
  s_timer_countdown_window = NULL;
}

// Click handlers
static void click_config_provider(void* context)
{
  window_single_click_subscribe(BUTTON_ID_UP, click_handler_up);
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler_select);
  window_single_click_subscribe(BUTTON_ID_DOWN, click_handler_down);
}

static void click_handler_up(ClickRecognizerRef recognizer, void* context)
{
  struct App_data* app_data = window_get_user_data(context);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  timer_reset(timer);
  cancel_app_timers();
  update_timer_countdown_text_layer(timer);
}

static void click_handler_select(ClickRecognizerRef recognizer, void* context)
{
  struct App_data* app_data = window_get_user_data(context);
  struct Settings* settings = timer_group_get_settings(app_data_get_timer_group(app_data, s_timer_group_index));
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  timer_update(timer);
  if (timer_is_elapsed(timer)) {
    if (settings_get_progress_style(settings) == PROGRESS_STYLE_WAIT_FOR_USER) {
      cancel_app_timers();
      timer_reset(timer);
      update_current_timer(app_data);
      timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
      timer_start(timer);
      update_timer_length_text_layer(timer);
      start_app_timer(0, timer_handler, app_data);
      return;
    }
    update_timer_countdown_text_layer(timer);
    return;
  }
  if (timer_is_running(timer)) {
    timer_pause(timer);
    cancel_app_timers();
  } else {
    timer_start(timer);
    update_timer_length_text_layer(timer);
    start_app_timer(0, timer_handler, app_data);
  }
}

static void click_handler_down(ClickRecognizerRef recognizer, void* context)
{
  struct App_data* app_data = window_get_user_data(context);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  timer_reset(timer);
  cancel_app_timers();
  timer_edit_window_push(app_data, s_timer_group_index, s_timer_index);
}

static void start_app_timer(int delay, AppTimerCallback app_timer_callback, void* data)
{
  s_app_timer_handle = app_timer_register(delay, app_timer_callback, data);
}

static void cancel_app_timers()
{
  if (s_app_timer_handle) {
    app_timer_cancel(s_app_timer_handle);
    s_app_timer_handle = NULL;
  }
  if (s_app_timer_vibrate_handle) {
    app_timer_cancel(s_app_timer_vibrate_handle);
    s_app_timer_vibrate_handle = NULL;
  }
}

static void timer_handler(void* data)
{
  s_app_timer_handle = NULL;
  struct App_data* app_data = data;
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  update_timer_countdown_text_layer(timer);
  if (!timer_is_running(timer)) {
    return;
  }
  struct Settings* settings = timer_group_get_settings(app_data_get_timer_group(app_data, s_timer_group_index));
  if (timer_is_elapsed(timer)) {
    vibrate_timer_handler(app_data);
  }
  if (timer_is_elapsed(timer) && settings_get_progress_style(settings) == PROGRESS_STYLE_AUTO) {
    cancel_app_timers();
    timer_reset(timer);
    update_current_timer(app_data);
    timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
    timer_start(timer);
    update_timer_length_text_layer(timer);
    update_timer_countdown_text_layer(timer);
  } else if (timer_is_elapsed(timer)) {
    return;
  }
  start_app_timer(MS_PER_SECOND, timer_handler, app_data);
}

static void vibrate_timer_handler(void* data)
{
  s_app_timer_vibrate_handle = NULL;
  vibes_double_pulse();
  struct App_data* app_data = data;
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  if (!timer_is_running(timer) || !timer_is_elapsed(timer)) {
    return;
  }
  s_app_timer_vibrate_handle = app_timer_register(NUDGE_INTERVAL_MS, vibrate_timer_handler, data);
}

static void update_current_timer(struct App_data* app_data)
{
  struct Timer_group* timer_group = app_data_get_timer_group(app_data, s_timer_group_index);
  if (timer_group_size(timer_group) <= 0) {
    return;
  }
  struct Settings* settings = timer_group_get_settings(timer_group);
  enum Repeat_style repeat_style = settings_get_repeat_style(settings);
  switch (repeat_style) {
    case REPEAT_STYLE_NONE:
      if (s_timer_index >= (timer_group_size(timer_group) - 1)) {
        return;
      }
      ++s_timer_index;
      return;
    case REPEAT_STYLE_SINGLE:
      return;
    case REPEAT_STYLE_GROUP:
      if (s_timer_index >= (timer_group_size(timer_group) - 1)) {
        s_timer_index = 0;
        return;
      }
      ++s_timer_index;
      return;
    case REPEAT_STYLE_INVALID: // intentional fall through
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Invalid repeat style: %d", repeat_style);
      return;
  }

}

static void update_timer_countdown_text_layer(struct Timer* timer)
{
  timer_update(timer);
  get_timer_text(s_timer_countdown_text_buffer, sizeof(s_timer_countdown_text_buffer),
    timer_get_field_remaining(timer, TIMER_FIELD_HOURS),
    timer_get_field_remaining(timer, TIMER_FIELD_MINUTES),
    timer_get_field_remaining(timer, TIMER_FIELD_SECONDS));
  text_layer_set_text(s_timer_countdown_text_layer, s_timer_countdown_text_buffer);
  layer_mark_dirty(text_layer_get_layer(s_timer_countdown_text_layer));
}

static void update_timer_length_text_layer(struct Timer* timer)
{
  get_timer_text(s_timer_length_text_buffer, sizeof(s_timer_length_text_buffer),
    timer_get_field(timer, TIMER_FIELD_HOURS),
    timer_get_field(timer, TIMER_FIELD_MINUTES),
    timer_get_field(timer, TIMER_FIELD_SECONDS));
  text_layer_set_text(s_timer_length_text_layer, s_timer_length_text_buffer);
  layer_mark_dirty(text_layer_get_layer(s_timer_length_text_layer));
}
