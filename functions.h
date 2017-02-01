#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "config.h"
#include "query.h"

struct Module;
struct Query;

typedef void(*queryCleanupFn)(struct Query *);
typedef void(*moduleCleanupFn)(struct Module *);
typedef int(*loadConfigFn)(struct Module *, const struct Config *, const char *);
typedef void(*processFn)(struct Module *, struct Query *);

#endif
