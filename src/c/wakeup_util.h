#ifndef WAKEUP_UTIL_H
#define WAKEUP_UTIL_H

struct Timer;

void timer_schedule_wakeup(struct Timer* timer);
void timer_cancel_wakeup(struct Timer* timer);

#endif /*WAKEUP_UTIL_H*/
