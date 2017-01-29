#pragma once

#define NUM_SETTINGS_FIELDS 3

struct Settings;

enum Settings_field {
  SETTINGS_FIELD_REPEAT_STYLE,
  SETTINGS_FIELD_PROGRESS_STYLE,
  SETTINGS_FIELD_VIBRATE_STYLE,
  SETTINGS_FIELD_INVALID
};

enum Repeat_style {
  REPEAT_STYLE_NONE,    // Progress to the next timer, but don't repeat the group
  REPEAT_STYLE_SINGLE,  // Repeat the current timer
  REPEAT_STYLE_GROUP,   // Progress to the next timer, repeating the group if at the last timer in the group
  REPEAT_STYLE_INVALID
};

enum Progress_style {
  PROGRESS_STYLE_NONE,
  PROGRESS_STYLE_AUTO,
  PROGRESS_STYLE_WAIT_FOR_USER,
  PROGRESS_STYLE_INVALID
};

enum Vibrate_style {
  VIBRATE_STYLE_NONE,
  VIBRATE_STYLE_NUDGE,
  VIBRATE_STYLE_CONTINUOUS,
  VIBRATE_STYLE_INVALID
};

struct Settings* settings_create();
void settings_destroy(struct Settings* settings);

struct Settings* settings_load();
void settings_save(struct Settings* settings);

void settings_set_repeat_style(struct Settings* settings, enum Repeat_style repeat_style);
enum Repeat_style settings_get_repeat_style(struct Settings* settings);

void settings_set_progress_style(struct Settings* settings, enum Progress_style progress_style);
enum Progress_style settings_get_progress_style(struct Settings* settings);

void settings_set_vibrate_style(struct Settings* settings, enum Vibrate_style vibrate_style);
enum Vibrate_style settings_get_vibrate_style(struct Settings* settings);

const char * settings_get_settings_field_text(enum Settings_field settings_field);
const char * settings_get_repeat_style_text(enum Repeat_style repeat_style);
const char * settings_get_progress_style_text(enum Progress_style progress_style);
const char * settings_get_vibrate_style_text(enum Vibrate_style vibrate_style);
