#pragma once

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
void* list_get(struct List* list, int index);

/*
Remove the item at the given index.
Caller is responsible for deleting pointed-to data first.
*/
void list_remove(struct List* list, int index);

/*
Functions that traverse the list, processesing each item in order.
*/

/*
Type of a function used by apply.
An apply function takes a data pointer as an argument, and returns void.
*/
typedef void (*List_apply_fp_t) (void* data_ptr);

/* Apply the supplied function to the data pointer in each item of the container. 
The contents of the container cannot be modified. */
void list_apply(const struct List* list, List_apply_fp_t func_ptr);
