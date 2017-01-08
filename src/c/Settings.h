#pragma once

struct Settings;

struct Settings* settings_create();
void settings_destroy(struct Settings* settings);

// void settings_persist_save(struct Settings settings);
// void settings_persist_load(struct Settings settings);

void settings_set_repeat_group(struct Settings* settings, int repeat_group);
int settings_get_repeat_group(struct Settings* settings);

void settings_set_auto_progress(struct Settings* settings, int auto_progress);
int settings_get_auto_progress(struct Settings* settings);

void settings_set_wait_for_user(struct Settings* settings, int wait_for_user);
int settings_get_wait_for_user(struct Settings* settings);