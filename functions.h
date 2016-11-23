#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "config.h"
#include "query.h"

typedef void(*cleanupFn)(void*);

struct Module;
struct Query;
typedef int(*loadConfigFn)(struct Module *, const struct Config *, const char *);
typedef void(*processFn)(struct Module *, struct Query *);


#endif
