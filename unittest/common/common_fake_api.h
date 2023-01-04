#ifndef COMMON_FAKE_API
#define COMMON_FAKE_API

#include  "stdint.h"
#include "wayland-client-protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

void custom_wl_list_init(struct wl_list *list);
void custom_wl_list_insert(struct wl_list *list, struct wl_list *elm);
void custom_wl_list_remove(struct wl_list *elm);
int  custom_wl_list_empty(const struct wl_list *list);
void custom_wl_array_init(struct wl_array *array);
void custom_wl_array_release(struct wl_array *array);
void *custom_wl_array_add(struct wl_array *array, size_t size);

#ifdef __cplusplus
}
#endif
#endif