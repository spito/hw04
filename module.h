#ifndef MODULE_H
#define MODULE_H

#include <stddef.h>

#include "query.h"
#include "functions.h"

#define MODULE_PRIVATE static

struct module {

    void *privateData;
    const char *name;

    loadConfigFn loadConfig;
    processFn process;
    processFn postProcess;

    moduleCleanupFn cleanup;

};

#endif
