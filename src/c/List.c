#include "List.h"
#include "Utility.h"
#include "assert.h"
#include "persist_util.h"

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

struct List* list_create()
{
	struct List* list = safe_alloc(sizeof(struct List));
  init_empty_intern(list);
  return list;
}

static void init_empty_intern(struct List* list)
{
  assert(list);
  list->array = safe_alloc(sizeof(void*) * DEFAULT_ARRAY_SIZE);
  list->allocated_size = DEFAULT_ARRAY_SIZE;
  list->size = 0;
}

void list_destroy(struct List* list)
{
  assert(list);
	free(list->array);
  list->array = NULL;
  free(list);
}

void list_clear(struct List* list)
{
  assert(list);
	free(list->array);
  init_empty_intern(list);
}

void list_add(struct List* list, void* item)
{
  assert(list);
  if (list->size >= list->allocated_size) {
    grow_list_intern(list);
  }
  list->array[list->size++] = item;
}

static void grow_list_intern(struct List* list)
{
  int new_allocated_size = GROW_FACTOR * (list->size + 1);
  void** new_array = safe_alloc(sizeof(void*) * new_allocated_size);
  for (int i = 0; i < list->size; ++i) {
    new_array[i] = list->array[i];
  }
  free(list->array);
  list->array = new_array;
  list->allocated_size = new_allocated_size;
}

int list_size(const struct List* list)
{
  assert(list);
  return list->size;
}

int list_empty(const struct List* list)
{
  assert(list);
  return list->size ? 0 : 1;
}

void* list_get(const struct List* list, int index)
{
  assert(list);
  if (!in_range(index, 0, list->size)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "invalid index");
    return NULL;
  }
  return list->array[index];
}

void list_remove(struct List* list, int index)
{
  assert(list);
  if (!in_range(index, 0, list->size)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "invalid index");
    return;
  }
  for (int i = index; i < list->size - 1; ++i) {
    list->array[i] = list->array[i + 1];
  }
  list->array[--list->size] = NULL;
}

void list_remove_ptr(struct List* list, void* data_ptr)
{
  assert(list);
  assert(data_ptr >= *(list->array));
  list_remove(list, data_ptr - *(list->array));
}

void list_for_each(const struct List* list, List_for_each_fp_t func_ptr)
{
  assert(list);
  for (int i = 0; i < list->size; ++i) {
    func_ptr(list->array[i]);
  }
}

void* list_find(const struct List* list, const void* data_ptr, List_compare_fp_t func_ptr)
{
  assert(list);
  return list_find_arg(list, data_ptr, func_ptr);
}

void* list_find_arg(const struct List* list, const void* arg_ptr, List_compare_arg_fp_t func_ptr)
{
  assert(list);
  for (int i = 0; i < list->size; ++i) {
    if (!func_ptr(arg_ptr, list->array[i])) {
      return list->array[i];
    }
  }
  return NULL;
}

struct List* list_load(List_load_item_fp_t func_ptr)
{
  assert(persist_exists(g_current_persist_key));
  int list_size = persist_read_int(g_current_persist_key++);
  struct List* list = list_create();
  for (int i = 0; i < list_size; ++i) {
    list_add(list, func_ptr());
  }
  return list;
}

void list_save(const struct List* list, List_for_each_fp_t func_ptr)
{
  assert(list);
  persist_write_int(g_current_persist_key++, list->size);
  list_for_each(list, func_ptr);
}
