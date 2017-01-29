#pragma once

#define PERSIST_VERSION_KEY 0
#define PERSIST_VERSION 1

extern int g_current_persist_key;

/*
Initialize persistence variables. Should be called before app data is loaded or
saved.
*/
void persist_init_load();
void persist_init_save();

/*
Should be called after app data is loaded.
*/
void persist_finish_load();

/*
Should be called after app data is saved.
*/
void persist_finish_save();
