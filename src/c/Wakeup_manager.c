#include "Wakeup_manager.h"
#include "List.h"
#include "Utility.h"
#include "persist_util.h"
#include "assert.h"
#include "Timer.h"
#include "App_data.h"
#include "timer_countdown_window.h"

#include <pebble.h>

// #define INT32_MAX 0x7FFFFFFFFFFFFFFF

static bool s_wakeup_service_subscribed = false;

struct Wakeup_manager {
  struct List* wakeup_data_list;
};
static void wakeup_manager_handle_wakeup_intern(struct Wakeup_manager* wakeup_manager, WakeupId wakeup_id, int32_t timer_id);

// Wakeup data
struct Wakeup_data;
static struct Wakeup_data* wakeup_data_create();
static void wakeup_data_destroy(struct Wakeup_data* wakeup_data);
static struct Wakeup_data* wakeup_data_load();
static void wakeup_data_save(const struct Wakeup_data* wakeup_data);
static void wakeup_data_set(struct Wakeup_data* wakeup_data, WakeupId wakeup_id, int timer_id);
static WakeupId wakeup_data_get_wakeup_id(const struct Wakeup_data* wakeup_data);
static int wakeup_data_compare_timer_id(const int* timer_id, const struct Wakeup_data* wakeup_data);

// Helpers
static void wakeup_handler(WakeupId wakeup_id, int32_t timer_id);
static void handle_wakeup_schedule_error(WakeupId error_id);
static struct Wakeup_data* get_wakeup_data_by_timer_id(const struct Wakeup_manager* wakeup_manager, int timer_id);

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
  assert(wakeup_manager);
  assert(timer);
  struct Wakeup_data* wakeup_data = get_wakeup_data_by_timer_id(wakeup_manager, timer_get_id(timer));
  if (wakeup_data) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Wakeup already scheduled for timer. Timer id: %d", timer_get_id(timer));
    return;
  }
  time_t wakeup_time = time(NULL) + timer_get_remaining_seconds(timer);
  WakeupId wakeup_id = wakeup_schedule(wakeup_time, timer_get_id(timer), true);
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
  struct Wakeup_data* wakeup_data = get_wakeup_data_by_timer_id(wakeup_manager, timer_get_id(timer));
  if (!wakeup_data) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Timer not scheduled. Timer id: %d", timer_get_id(timer));
    return;
  }
  wakeup_cancel(wakeup_data_get_wakeup_id(wakeup_data));
  list_remove_ptr(wakeup_manager->wakeup_data_list, wakeup_data);
  wakeup_data_destroy(wakeup_data);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Wakeup canceled. Timer id: %d", timer_get_id(timer));
}

static void wakeup_manager_handle_wakeup_intern(struct Wakeup_manager* wakeup_manager, WakeupId wakeup_id, int32_t timer_id)
{
  wakeup_manager_cancel(wakeup_manager, app_data_get_timer_by_id(app_data_get(), timer_id));
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

static void handle_wakeup_schedule_error(WakeupId error_id)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to schedule wakeup: %d", (int)error_id);
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
