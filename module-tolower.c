#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "log.h"
#include "module-tolower.h"

MODULE_PRIVATE
void responseCleanup(struct query *query)
{
    free(query->response);
    query->response = NULL;
    query->responseLength = 0;
}

MODULE_PRIVATE
void process(struct module *module, struct query *query)
{
    (void)module;

    if (!query->query || !query->queryLength) {
        query->responseCode = RCInvalidInput;
        return;
    }

    query->responseLength = query->queryLength;
    query->response = (char *)malloc(query->responseLength + 1);
    if (!query->response) {
        LOG(LFatal, "Allocation failed (%zu bytes)", query->responseLength + 1);
        query->responseCode = RCError;
        return;
    }

    query->responseCleanup = responseCleanup;

    char *out = query->response;
    for (const char *in = query->query; *in; ++in, ++out) {
        *out = tolower(*in);
    }
    *out = '\0';
    query->responseCode = RCSuccess;
}

void moduleToLower(struct module *module)
{
    module->privateData = NULL;
    module->name = "tolower";
    module->loadConfig = NULL;
    module->process = process;
    module->postProcess = NULL;
    module->cleanup = NULL;
}
