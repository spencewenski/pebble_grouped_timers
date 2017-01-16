#pragma once

#include <pebble.h>

#ifdef NDEBUG
#define assert(e) ((void)0)
#else
#define assert(e)  \
    ((void) ((e) ? ((void)0) : __assert (#e, __FILE__, __LINE__)))
#define __assert(e, file, line) \
    ((void)APP_LOG(APP_LOG_LEVEL_ERROR, "%s:%u: failed assertion '%s'\n", file, line, e), window_stack_pop_all(false))
#endif
