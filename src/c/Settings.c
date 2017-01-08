#include "Settings.h"
#include "Utility.h"

#include <pebble.h>

struct Settings {
  int repeat_group;    /* repeat the group after the last timer completes. */
  int auto_progress;   /* Automatically start the next timer after the current one completes. */
  int wait_for_user;   /* Wait for the user to stop the current timer before starting the next one. */
};

struct Settings* settings_create() {
  return safe_alloc(sizeof(struct Settings));
}

void settings_destroy(struct Settings* settings) {
  free(settings);
}