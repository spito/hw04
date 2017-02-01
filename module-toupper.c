#include <stdlib.h>
#include <ctype.h>

#include "config.h"
#include "log.h"
#include "module-toupper.h"

MODULE_PRIVATE
void responseCleanup(struct Query *query) {
    free(query->response);
    query->response = NULL;
    query->responseLength = 0;
}

MODULE_PRIVATE
void process(struct Module *module, struct Query *query) {
    (void)module;

    if (!query->query || !query->queryLength) {
        query->responseCode = RC_INVALID_INPUT;
        return;
    }

    query->responseLength = query->queryLength;
    query->response = (char *)malloc(query->responseLength + 1);
    if (!query->response) {
        LOG(L_FATAL, "Allocation failed (%zu bytes)", query->responseLength + 1);
        query->responseCode = RC_ERROR;
        return;
    }

    query->responseCleanup = responseCleanup;

    char *out = query->response;
    for (const char *in = query->query; *in; ++in, ++out) {
        *out = toupper(*in);
    }
    *out = '\0';
    query->responseCode = RC_SUCCESS;
}

void moduleToUpper(struct Module *module) {
    module->privateData = NULL;
    module->name = "toupper";
    module->loadConfig = NULL;
    module->process = process;
    module->postProcess = NULL;
    module->cleanup = NULL;
}
