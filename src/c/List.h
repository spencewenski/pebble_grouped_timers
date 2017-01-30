#ifndef LIST_H
#define LIST_H

struct List;

/*
Create a new, empty List.
*/
struct List* list_create();

/*
Destroy the list and its items.
Caller is responsible for deleting all pointed-to data before calling this function. 
After this call, the list pointer value must not be used again.
*/
void list_destroy(struct List* list);

/*
Clear the list.
Caller is responsible for deleting pointed-to data first.
*/
void list_clear(struct List* list);

/*
Add the given item to the back of the list.
*/
void list_add(struct List* list, void* item);

/*
Get the size of the list.
Return -1 if an error occurs.
*/
int list_size(const struct List* list);

/*
Return non-zero (true) if the list is empty, zero (false) if the list is non-empty.
Return -1 if an error occurs.
*/
int list_empty(const struct List* list);

/*
Get the item at the given index.
*/
void* list_get(const struct List* list, int index);

/*
Remove the item at the given index.
Caller is responsible for deleting pointed-to data first.
*/
void list_remove(struct List* list, int index);

/*
Type of a function used by apply.
An apply function takes a data pointer as an argument, and returns void.
*/
typedef void (*List_apply_fp_t) (void* data);

/* Apply the supplied function to the data pointer in each item of the container. 
The contents of the container cannot be modified. */
void list_apply(const struct List* list, List_apply_fp_t func_ptr);

/*
Type of function used to load items to be added to the list.
A load function returns a pointer to an item to add to the list.
*/
typedef void* (*List_load_item_fp_t) ();

/*
Use the supplied function to load items and add them to the list.
*/
struct List* list_load(List_load_item_fp_t func_ptr);

/*
Save the list. Items are saved by applying the given function pointer to each
item in the list in order.
*/
void list_save(const struct List* list, List_apply_fp_t func_ptr);

#endif /*LIST_H*/
