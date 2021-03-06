#include "Settings.h"
#include "Utility.h"
#include "assert.h"
#include "persist_util.h"

#include <pebble.h>

struct Settings {
  enum Repeat_style repeat_style;     /* repeat the group after the last timer completes. */
  enum Progress_style progress_style; /* Automatically start the next timer after the current one completes. */
  enum Vibrate_style vibrate_style;   /* (Only if wait for user) Continuous or nudge every minute. */
};

struct Settings* settings_create()
{
  struct Settings* settings = safe_alloc(sizeof(struct Settings));
  settings->repeat_style = REPEAT_STYLE_NONE;
  settings->progress_style = PROGRESS_STYLE_NONE;
  settings->vibrate_style = VIBRATE_STYLE_NONE;
  return settings;
}

void settings_destroy(struct Settings* settings)
{
  free(settings);
}

struct Settings* settings_load()
{
  struct Settings* settings = safe_alloc(sizeof(struct Settings));
  persist_read_data(g_current_persist_key++, settings, sizeof(struct Settings));
  return settings;
}

void settings_save(const struct Settings* settings)
{
  assert(settings);
  persist_write_data(g_current_persist_key++, settings, sizeof(struct Settings));
}

void settings_set_repeat_style(struct Settings* settings, enum Repeat_style repeat_style)
{
  assert(settings);
  settings->repeat_style = repeat_style;
}

enum Repeat_style settings_get_repeat_style(const struct Settings* settings)
{
  assert(settings);
  return settings->repeat_style;
}

void settings_set_progress_style(struct Settings* settings, enum Progress_style progress_style)
{
  assert(settings);
  settings->progress_style = progress_style;
}

enum Progress_style settings_get_progress_style(const struct Settings* settings)
{
  assert(settings);
  return settings->progress_style;
}

void settings_set_vibrate_style(struct Settings* settings, enum Vibrate_style vibrate_style)
{
  assert(settings);
  settings->vibrate_style = vibrate_style;
}

enum Vibrate_style settings_get_vibrate_style(const struct Settings* settings)
{
  assert(settings);
  return settings->vibrate_style;
}

const char * settings_get_settings_field_text(enum Settings_field settings_field)
{
  switch (settings_field) {
    case SETTINGS_FIELD_REPEAT_STYLE:
      return "Repeat Style";
    case SETTINGS_FIELD_PROGRESS_STYLE:
      return "Progress Style";
    case SETTINGS_FIELD_VIBRATE_STYLE:
      return "Vibrate Style";
    case SETTINGS_FIELD_INVALID: // intentional fall through
    default:
      return "";
  }
}

const char * settings_get_repeat_style_text(enum Repeat_style repeat_style)
{
  switch (repeat_style) {
    case REPEAT_STYLE_NONE:
      return "Repeat none";
    case REPEAT_STYLE_SINGLE:
      return "Repeat single";
    case REPEAT_STYLE_GROUP:
      return "Repeat group";
    case REPEAT_STYLE_INVALID: // intentional fall through
    default:
      return "";
  }
}

const char * settings_get_progress_style_text(enum Progress_style progress_style)
{
  switch (progress_style) {
    case PROGRESS_STYLE_NONE:
      return "Don't start next";
    case PROGRESS_STYLE_AUTO:
      return "Auto start next";
    case PROGRESS_STYLE_WAIT_FOR_USER:
      return "Wait for user";
    case PROGRESS_STYLE_INVALID: // intentional fall through
    default:
      return "";
  }
}

const char * settings_get_vibrate_style_text(enum Vibrate_style vibrate_style)
{
  switch (vibrate_style) {
    case VIBRATE_STYLE_NONE:
      return "Don't vibrate";
    case VIBRATE_STYLE_NUDGE:
      return "Nudge";
    case VIBRATE_STYLE_CONTINUOUS:
      return "Continuous";
    case VIBRATE_STYLE_INVALID: // intentional fall through
    default:
      return "";
  }
}
