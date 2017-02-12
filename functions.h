#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "config.h"
#include "query.h"

struct module;
struct query;

typedef void(*queryCleanupFn)(struct query *);
typedef void(*moduleCleanupFn)(struct module *);
typedef int(*loadConfigFn)(struct module *, const struct config *, const char *);
typedef void(*processFn)(struct module *, struct query *);

#endif
