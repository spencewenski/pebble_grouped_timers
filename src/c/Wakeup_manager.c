#include "Wakeup_manager.h"
#include "List.h"
#include "Utility.h"
#include "persist_util.h"
#include "assert.h"
#include "Timer.h"
#include "App_data.h"
#include "timer_countdown_window.h"
#include "globals.h"

#include <pebble.h>

// #define INT32_MAX 0x7FFFFFFFFFFFFFFF

static bool s_wakeup_service_subscribed = false;

struct Wakeup_manager {
  struct List* wakeup_data_list;
};
static void wakeup_manager_handle_wakeup_intern(struct Wakeup_manager* wakeup_manager, WakeupId wakeup_id, int32_t timer_id);
// seconds -> number of seconds in the future to set the wakeup
static void wakeup_manager_schedule_intern(struct Wakeup_manager* wakeup_manager, const struct Timer* timer, int seconds);
static void wakeup_manager_cancel_intern(struct Wakeup_manager* wakeup_manager, WakeupId wakeup_id, int32_t timer_id);

// Wakeup data
struct Wakeup_data;
static struct Wakeup_data* wakeup_data_create();
static void wakeup_data_destroy(struct Wakeup_data* wakeup_data);
static struct Wakeup_data* wakeup_data_load();
static void wakeup_data_save(const struct Wakeup_data* wakeup_data);
static void wakeup_data_set(struct Wakeup_data* wakeup_data, WakeupId wakeup_id, int timer_id);
static WakeupId wakeup_data_get_wakeup_id(const struct Wakeup_data* wakeup_data);
static int wakeup_data_compare_timer_id(const int* timer_id, const struct Wakeup_data* wakeup_data);
static int wakeup_data_compare_wakeup_id(const WakeupId* wakeup_id, const struct Wakeup_data* wakeup_data);

// Helpers
static void wakeup_handler(WakeupId wakeup_id, int32_t timer_id);
static void handle_wakeup_schedule_error(WakeupId error_id);
static struct Wakeup_data* get_wakeup_data_by_timer_id(const struct Wakeup_manager* wakeup_manager, int timer_id);
static struct Wakeup_data* get_wakeup_data_by_wakeup_id(const struct Wakeup_manager* wakeup_manager, WakeupId wakeup_id);

struct Wakeup_manager* wakeup_manager_create()
{
  struct Wakeup_manager* wakeup_manager = safe_alloc(sizeof(struct Wakeup_manager));
  wakeup_manager->wakeup_data_list = list_create();
  if (!s_wakeup_service_subscribed) {
    wakeup_service_subscribe(wakeup_handler);
    s_wakeup_service_subscribed = true;
  }
  return wakeup_manager;
}

void wakeup_manager_destroy(struct Wakeup_manager* wakeup_manager)
{
  assert(wakeup_manager);
  list_for_each(wakeup_manager->wakeup_data_list, (List_for_each_fp_t) wakeup_data_destroy);
  list_destroy(wakeup_manager->wakeup_data_list);
  free(wakeup_manager);
}

struct Wakeup_manager* wakeup_manager_load()
{
  struct Wakeup_manager* wakeup_manager = safe_alloc(sizeof(struct Wakeup_manager));
  wakeup_manager->wakeup_data_list = list_load((List_load_item_fp_t) wakeup_data_load);
  if (!s_wakeup_service_subscribed) {
    wakeup_service_subscribe(wakeup_handler);
    s_wakeup_service_subscribed = true;
  }
  return wakeup_manager;
}

void wakeup_manager_save(const struct Wakeup_manager* wakeup_manager)
{
  assert(wakeup_manager);
  list_save(wakeup_manager->wakeup_data_list, (List_for_each_fp_t) wakeup_data_save);
}

void wakeup_manager_handle_wakeup(struct Wakeup_manager* wakeup_manager)
{
  if (launch_reason() != APP_LAUNCH_WAKEUP) {
    return;
  }
  WakeupId wakeup_id = 0;
  int32_t timer_id = 0;
  wakeup_get_launch_event(&wakeup_id, &timer_id);
  wakeup_manager_handle_wakeup_intern(wakeup_manager, wakeup_id, timer_id);
}

void wakeup_manager_schedule(struct Wakeup_manager* wakeup_manager, const struct Timer* timer)
{
  assert(timer);
  wakeup_manager_schedule_intern(wakeup_manager, timer, timer_get_remaining_seconds(timer));
}

void wakeup_manager_schedule_nudge(struct Wakeup_manager* wakeup_manager, const struct Timer* timer)
{
  wakeup_manager_schedule_intern(wakeup_manager, timer, NUDGE_INTERVAL_SECOND);
}

static void wakeup_manager_schedule_intern(struct Wakeup_manager* wakeup_manager, const struct Timer* timer, int seconds)
{
  assert(wakeup_manager);
  assert(timer);
  struct Wakeup_data* wakeup_data = get_wakeup_data_by_timer_id(wakeup_manager, timer_get_id(timer));
  if (wakeup_data) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Wakeup already scheduled for timer. Timer id: %d", timer_get_id(timer));
    return;
  }
  WakeupId wakeup_id = wakeup_schedule(time(NULL) + seconds, timer_get_id(timer), false);
  if (wakeup_id < 0) {
    handle_wakeup_schedule_error(wakeup_id);
    return;
  }
  wakeup_data = wakeup_data_create();
  wakeup_data_set(wakeup_data, wakeup_id, timer_get_id(timer));
  list_add(wakeup_manager->wakeup_data_list, wakeup_data);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Wakeup scheduled. Timer id: %d", timer_get_id(timer));
}

void wakeup_manager_cancel(struct Wakeup_manager* wakeup_manager, const struct Timer* timer)
{
  assert(wakeup_manager);
  assert(timer);
  int timer_id = timer_get_id(timer);
  struct Wakeup_data* wakeup_data = get_wakeup_data_by_timer_id(wakeup_manager, timer_id);
  if (!wakeup_data) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Wakeup not scheduled. Timer id: %d", (int) timer_id);
    return;
  }
  wakeup_manager_cancel_intern(wakeup_manager, wakeup_data_get_wakeup_id(wakeup_data), timer_id);
}

static void wakeup_manager_cancel_intern(struct Wakeup_manager* wakeup_manager, WakeupId wakeup_id, int32_t timer_id)
{
  struct Wakeup_data* wakeup_data = get_wakeup_data_by_wakeup_id(wakeup_manager, wakeup_id);
  if (!wakeup_data) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Wakeup not scheduled. Timer id: %d", (int) timer_id);
    return;
  }
  wakeup_cancel(wakeup_id);
  list_remove_ptr(wakeup_manager->wakeup_data_list, wakeup_data);
  wakeup_data_destroy(wakeup_data);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Wakeup canceled. Timer id: %d", (int) timer_id);
}

static void wakeup_manager_handle_wakeup_intern(struct Wakeup_manager* wakeup_manager, WakeupId wakeup_id, int32_t timer_id)
{
  wakeup_manager_cancel_intern(wakeup_manager, wakeup_id, timer_id);
  timer_countdown_window_push_id(timer_id);
}

// Helpers
static void wakeup_handler(WakeupId wakeup_id, int32_t timer_id)
{
  struct Wakeup_manager* wakeup_manager = app_data_get_wakeup_manager(app_data_get());
  wakeup_manager_handle_wakeup_intern(wakeup_manager, wakeup_id, timer_id);
}


static struct Wakeup_data* get_wakeup_data_by_timer_id(const struct Wakeup_manager* wakeup_manager, int timer_id)
{
  assert(wakeup_manager);
  return list_find(wakeup_manager->wakeup_data_list, &timer_id,
    (List_compare_arg_fp_t) wakeup_data_compare_timer_id);
}

static struct Wakeup_data* get_wakeup_data_by_wakeup_id(const struct Wakeup_manager* wakeup_manager, WakeupId wakeup_id)
{
  assert(wakeup_manager);
  return list_find(wakeup_manager->wakeup_data_list, &wakeup_id,
    (List_compare_arg_fp_t) wakeup_data_compare_wakeup_id);
}

static void handle_wakeup_schedule_error(WakeupId error_id)
{
  switch (error_id) {
    case E_RANGE:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup, E_RANGE");
      break;
    case E_INVALID_ARGUMENT:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup, E_INVALID_ARGUMENT");
      break;
    case E_OUT_OF_RESOURCES:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup, E_OUT_OF_RESOURCES");
      break;
    case E_INTERNAL:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup, E_INTERNAL");
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup, unkown error: %d", (int)error_id);
  }
}

// Wakeup_data
struct Wakeup_data {
  WakeupId wakeup_id;
  int timer_id;
};

#define INVALID_WAKEUP_ID -1
#define INVALID_TIMER_ID -1

static struct Wakeup_data* wakeup_data_create()
{
  struct Wakeup_data* wakeup_data = safe_alloc(sizeof(struct Wakeup_data));
  wakeup_data->wakeup_id = INVALID_WAKEUP_ID;
  wakeup_data->timer_id = INVALID_TIMER_ID;
  return wakeup_data;
}

static void wakeup_data_destroy(struct Wakeup_data* wakeup_data)
{
  free(wakeup_data);
}

static struct Wakeup_data* wakeup_data_load()
{
  struct Wakeup_data* wakeup_data = safe_alloc(sizeof(struct Wakeup_data));
  persist_read_data(g_current_persist_key++, wakeup_data, sizeof(struct Wakeup_data));
  return wakeup_data;
}

static void wakeup_data_save(const struct Wakeup_data* wakeup_data)
{
  assert(wakeup_data);
  persist_write_data(g_current_persist_key++, wakeup_data, sizeof(struct Wakeup_data));
}

static void wakeup_data_set(struct Wakeup_data* wakeup_data, WakeupId wakeup_id, int timer_id)
{
  assert(wakeup_data);
  assert(timer_id <= INT32_MAX);
  wakeup_data->wakeup_id = wakeup_id;
  wakeup_data->timer_id = timer_id;
}

static WakeupId wakeup_data_get_wakeup_id(const struct Wakeup_data* wakeup_data)
{
  assert(wakeup_data);
  return wakeup_data->wakeup_id;
}

static int wakeup_data_compare_timer_id(const int* timer_id, const struct Wakeup_data* wakeup_data)
{
  assert(timer_id);
  assert(wakeup_data);
  return *timer_id - wakeup_data->timer_id;
}

static int wakeup_data_compare_wakeup_id(const WakeupId* wakeup_id, const struct Wakeup_data* wakeup_data)
{
  assert(wakeup_id);
  assert(wakeup_data);
  return *wakeup_id - wakeup_data->wakeup_id;
}
