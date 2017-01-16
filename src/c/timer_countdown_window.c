#include "timer_countdown_window.h"
#include "globals.h"
#include "App_data.h"
#include "Timer.h"
#include "Settings.h"
#include "List.h"

#include <pebble.h>

static Window* s_timer_countdown_window;
static TextLayer* s_timer_text_layer;
static int s_timer_group_index;
static int s_timer_index;
static AppTimer* s_app_timer_handle;

static char s_timer_text_buffer[TIMER_TEXT_LENGTH];

// Window Handlers
void window_load_handler(Window* window);
void window_unload_handler(Window* window);

// Timer display
static void update_timer_text_layer(struct Timer* timer);

// Click handlers
static void click_config_provider(void* context);
static void click_handler_select(ClickRecognizerRef recognizer, void* context);

// Timer handler
static void start_app_timer(int delay, AppTimerCallback app_timer_callback, void* data);
static void cancel_app_timer();
static void timer_handler(void* data);

// Helpers
static void update_current_timer(struct App_data* app_data);

void timer_countdown_window_push(struct App_data* app_data, int timer_group_index, int timer_index) {
  s_timer_countdown_window = window_create();
  
  if (!s_timer_countdown_window) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Null main window");
    return;
  }
  
  s_timer_group_index = timer_group_index;
  s_timer_index = timer_index;
  
  window_set_user_data(s_timer_countdown_window, app_data);
  
  window_set_window_handlers(s_timer_countdown_window, (WindowHandlers) {
    .load = window_load_handler,
    .unload = window_unload_handler
  });
  
  window_stack_push(s_timer_countdown_window, false);
}

void window_load_handler(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  window_set_click_config_provider(window, click_config_provider);

  GRect window_bounds = layer_get_bounds(window_layer);
  
  // Setup other static variables
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
  start_app_timer(0, timer_handler, app_data);
}

void window_unload_handler(Window* window) {
  cancel_app_timer();

  text_layer_destroy(s_timer_text_layer);
  s_timer_text_layer = NULL;
  
  window_destroy(s_timer_countdown_window);
  s_timer_countdown_window = NULL;
}

// Click handlers
static void click_config_provider(void* context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler_select);
}

static void click_handler_select(ClickRecognizerRef recognizer, void* context) {
  struct App_data* app_data = window_get_user_data(context);
  struct Settings* settings = app_data_get_settings(app_data);
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  timer_update(timer);
  if (timer_is_elapsed(timer)) {
    timer_reset(timer);
    if (settings_get_progress_style(settings) == PROGRESS_STYLE_WAIT_FOR_USER) {
      update_current_timer(app_data);
      timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
      timer_start(timer);
      start_app_timer(0, timer_handler, app_data);
      return;
    }
    update_timer_text_layer(timer);
    return;
  }
  if (timer_is_running(timer)) {
    timer_pause(timer);
    cancel_app_timer();
  } else {
    timer_start(timer);
    start_app_timer(0, timer_handler, app_data);
  }
}

static void start_app_timer(int delay, AppTimerCallback app_timer_callback, void* data) {
  s_app_timer_handle = app_timer_register(delay, app_timer_callback, data);
}

static void cancel_app_timer() {
  if (!s_app_timer_handle) {
    return;
  }
  app_timer_cancel(s_app_timer_handle);
  s_app_timer_handle = NULL;
}

static void timer_handler(void* data) {
  s_app_timer_handle = NULL;
  struct App_data* app_data = data;
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  update_timer_text_layer(timer);
  if (!timer_is_running(timer)) {
    return;
  }
  struct Settings* settings = app_data_get_settings(app_data);
  if (timer_is_elapsed(timer) && settings_get_progress_style(settings) == PROGRESS_STYLE_AUTO) {
    timer_reset(timer);
    update_current_timer(app_data);
    timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
    timer_start(timer);
    update_timer_text_layer(timer);
  } else if (timer_is_elapsed(timer)) {
    return;
  }
  start_app_timer(MS_PER_SECOND, timer_handler, app_data);
}

static void update_current_timer(struct App_data* app_data) {
  struct List* timer_group = app_data_get_timer_group(app_data, s_timer_group_index);
  if (list_size(timer_group) <= 0) {
    return;
  }
  struct Settings* settings = app_data_get_settings(app_data);
  enum Repeat_style repeat_style = settings_get_repeat_style(settings);
  switch (repeat_style) {
    case REPEAT_STYLE_NONE:
      if (s_timer_index >= (list_size(timer_group) - 1)) {
        return;
      }
      ++s_timer_index;
      return;
    case REPEAT_STYLE_SINGLE:
      return;
    case REPEAT_STYLE_GROUP:
      if (s_timer_index >= (list_size(timer_group) - 1)) {
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

static void update_timer_text_layer(struct Timer* timer) {
  timer_update(timer);
  if (timer_get_field_remaining(timer, TIMER_FIELD_HOURS) > 0) {
    snprintf(s_timer_text_buffer, sizeof(s_timer_text_buffer), "%.2d:%.2d:%.2d",
            timer_get_field_remaining(timer, TIMER_FIELD_HOURS),
            timer_get_field_remaining(timer, TIMER_FIELD_MINUTES),
            timer_get_field_remaining(timer, TIMER_FIELD_SECONDS));
  } else {
    snprintf(s_timer_text_buffer, sizeof(s_timer_text_buffer), "%.2d:%.2d",
            timer_get_field_remaining(timer, TIMER_FIELD_MINUTES),
            timer_get_field_remaining(timer, TIMER_FIELD_SECONDS));
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Timer value - %s", s_timer_text_buffer);
  text_layer_set_text(s_timer_text_layer, s_timer_text_buffer);
  layer_mark_dirty(text_layer_get_layer(s_timer_text_layer));
}