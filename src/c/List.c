#include "List.h"
#include "Utility.h"

#include <pebble.h>

#define DEFAULT_ARRAY_SIZE 3
#define GROW_FACTOR 2

struct List {
  void** array;
  int allocated_size;
  int size;
};

static void init_empty_intern(struct List* list);
static void grow_list_intern(struct List* list);

struct List* list_create() {
	struct List* list = safe_alloc(sizeof(struct List));
  init_empty_intern(list);
  return list;
}

static void init_empty_intern(struct List* list) {
  if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return;
  }
  list->array = safe_alloc(sizeof(void*) * DEFAULT_ARRAY_SIZE);
  list->allocated_size = DEFAULT_ARRAY_SIZE;
  list->size = 0;
}

void list_destroy(struct List* list) {
  if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return;
  }
	free(list->array);
  list->array = NULL;
  free(list);
}

void list_clear(struct List* list) {
  if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return;
  }
	free(list->array);
  init_empty_intern(list);
}

void list_add(struct List* list, void* item) {
  if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return;
  }
  if (list->size >= list->allocated_size) {
    grow_list_intern(list);
  }
  list->array[list->size++] = item;
}

static void grow_list_intern(struct List* list) {
  int new_allocated_size = GROW_FACTOR * (list->size + 1);
  void** new_array = safe_alloc(sizeof(void*) * new_allocated_size);
  for (int i = 0; i < list->size; ++i) {
    new_array[i] = list->array[i];
  }
  free(list->array);
  list->array = new_array;
  list->allocated_size = new_allocated_size;
}

int list_size(const struct List* list) {
	if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return -1;
  }
  return list->size;
}

int list_empty(const struct List* list) {
	if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return -1;
  }
  return list->size ? 0 : 1;
}

void* list_get(struct List* list, int index) {
  if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return NULL;
  }
  if (!in_range(index, 0, list->size)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "invalid index");
    return NULL;
  }
  return *((void**) (list->array[index]));
}

void list_remove(struct List* list, int index) {
  if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return;
  }
  if (!in_range(index, 0, list->size)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "invalid index");
    return;
  }
  for (int i = index; i < list->size - 1; ++i) {
    list->array[index] = list->array[i + 1];
  }
  list->array[list->size--] = NULL;
}

void list_apply(const struct List* list, List_apply_fp_t func_ptr) {
  if (!list) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "null list pointer");
    return;
  }
  for (int i = 0; i < list->size; ++i) {
    func_ptr(list->array[i]);
  }
}