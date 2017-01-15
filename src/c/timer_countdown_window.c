#include "timer_countdown_window.h"
#include "globals.h"
#include "App_data.h"
#include "Timer.h"

#include <pebble.h>

static Window* s_timer_countdown_window;
static TextLayer* s_timer_text_layer;
static int s_timer_group_index;
static int s_timer_index;

static char s_timer_text_buffer[TIMER_TEXT_LENGTH];

// Window Handlers
void window_load_handler(Window* window);
void window_unload_handler(Window* window);

// Timer display
static void update_timer_text_layer(struct Timer* timer);

// Click handlers
static void click_config_provider(void* context);
static void click_handler_select(ClickRecognizerRef recognizer, void* context);

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
  struct Timer* timer = app_data_get_timer(app_data, s_timer_group_index, s_timer_index);
  update_timer_text_layer(timer);  
}

void window_unload_handler(Window* window) {
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
  // Start/pause/reset
}

static void update_timer_text_layer(struct Timer* timer) {
  if (timer_get_field(timer, TIMER_FIELD_HOURS) > 0) {
    snprintf(s_timer_text_buffer, sizeof(s_timer_text_buffer), "%.2d:%.2d:%.2d",
            timer_get_field(timer, TIMER_FIELD_HOURS),
            timer_get_field(timer, TIMER_FIELD_MINUTES),
            timer_get_field(timer, TIMER_FIELD_SECONDS));
  } else {
    snprintf(s_timer_text_buffer, sizeof(s_timer_text_buffer), "%.2d:%.2d",
            timer_get_field(timer, TIMER_FIELD_MINUTES),
            timer_get_field(timer, TIMER_FIELD_SECONDS));
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Timer value - %s", s_timer_text_buffer);
  text_layer_set_text(s_timer_text_layer, s_timer_text_buffer);
  layer_mark_dirty(text_layer_get_layer(s_timer_text_layer));
}