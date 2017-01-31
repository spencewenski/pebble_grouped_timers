#include "draw_utility.h"
#include "assert.h"

#include <pebble.h>

void get_timer_text(char* buf, int buf_size, int hours, int minutes,
  int seconds)
{
  assert(buf);
  if (hours > 0) {
    snprintf(buf, buf_size, "%d:%.2d:%.2d", hours, minutes, seconds);
  } else if (minutes > 0) {
    snprintf(buf, buf_size, "%d:%.2d", minutes, seconds);
  } else {
    snprintf(buf, buf_size, ":%.2d", seconds);
  }
}
