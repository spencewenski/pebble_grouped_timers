#ifndef WAKEUP_MANAGER_H
#define WAKEUP_MANAGER_H

struct Wakeup_manager;
struct Timer;

struct Wakeup_manager* wakeup_manager_create();
void wakeup_manager_destroy(struct Wakeup_manager* wakeup_manager);

struct Wakeup_manager* wakeup_manager_load();
void wakeup_manager_save(const struct Wakeup_manager* wakeup_manager);

void wakeup_manager_handle_wakeup(struct Wakeup_manager* wakeup_manager);

void wakeup_manager_schedule(struct Wakeup_manager* wakeup_manager, const struct Timer* timer);
void wakeup_manager_schedule_nudge(struct Wakeup_manager* wakeup_manager, const struct Timer* timer);
void wakeup_manager_cancel(struct Wakeup_manager* wakeup_manager, const struct Timer* timer);

#endif /*WAKEUP_MANAGER_H*/
