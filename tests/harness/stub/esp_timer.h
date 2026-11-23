#pragma once
#include <cstdint>
typedef struct esp_timer* esp_timer_handle_t;
typedef struct{void(*callback)(void*);void* arg;int dispatch_method;const char* name;bool skip_unhandled_events;} esp_timer_create_args_t;
int esp_timer_create(const esp_timer_create_args_t*,esp_timer_handle_t*); int esp_timer_start_periodic(esp_timer_handle_t,uint64_t); int64_t esp_timer_get_time();
