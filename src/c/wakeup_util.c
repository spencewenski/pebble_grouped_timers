#include "wakeup_util.h"
#include "Wakeup_manager.h"
#include "App_data.h"
#include "Timer.h"
#include "assert.h"

void timer_cancel_wakeup(struct Timer* timer)
{
  assert(timer);
  wakeup_manager_cancel(app_data_get_wakeup_manager(app_data_get()), timer);
}

void timer_schedule_wakeup(struct Timer* timer)
{
  assert(timer);
  wakeup_manager_schedule(app_data_get_wakeup_manager(app_data_get()), timer);
}
