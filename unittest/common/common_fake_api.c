#include "common_fake_api.h"
#include <string.h>
#include <stdlib.h>

/* Invalid memory address */
#define WL_ARRAY_POISON_PTR (void *) 4

void custom_wl_list_init(struct wl_list *list)
{
    list->prev = list;
    list->next = list;
}

void custom_wl_list_insert(struct wl_list *list, struct wl_list *elm)
{
    elm->prev = list;
    elm->next = list->next;
    list->next = elm;
    elm->next->prev = elm;
}

void custom_wl_list_remove(struct wl_list *elm)
{
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
    elm->next = NULL;
    elm->prev = NULL;
}

int custom_wl_list_empty(const struct wl_list *list)
{
    return list->next == list;
}

void custom_wl_array_init(struct wl_array *array)
{
    memset(array, 0, sizeof *array);
}

void custom_wl_array_release(struct wl_array *array)
{
    free(array->data);
    array->data = WL_ARRAY_POISON_PTR;
}

void *custom_wl_array_add(struct wl_array *array, size_t size)
{
    size_t alloc;
    void *data, *p;

    if (array->alloc > 0)
        alloc = array->alloc;
    else
        alloc = 16;

    while (alloc < array->size + size)
        alloc *= 2;

    if (array->alloc < alloc) {
        if (array->alloc > 0)
            data = realloc(array->data, alloc);
        else
            data = malloc(alloc);

        if (data == NULL)
            return NULL;
        array->data = data;
        array->alloc = alloc;
    }

    p = (char *)array->data + array->size;
    array->size += size;

    return p;
}
