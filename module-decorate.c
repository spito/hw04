#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "log.h"
#include "module-decorate.h"

static struct {
    const char *name;
    int code;
} table[] = {
    {"black", 30},
    {"red", 31},
    {"green", 32},
    {"yellow", 33},
    {"blue", 34},
    {"magneta", 35},
    {"cyan", 36},
    {"light grey", 37},
    {"default", 39}
};

struct Decoration {
    const char *prefix;
    size_t prefixLength;
    const char *suffix;
    size_t suffixLength;
};

MODULE_PRIVATE
int loadConfig(struct Module *module, const struct Config *cfg, const char *section) {
    struct Decoration *decoration = (struct Decoration *)module->privateData;
    if (!decoration) {
        LOG(L_ERROR, "Corrupted module");
        return -1;
    }

    int rv;
    const char *color;
    if ((rv = configValue(cfg, section, "Color", CFG_STRING, &color))) {
        LOG(L_WARN, "Could not read value Color, using default = default");
        color = "default";
    }

    int bold;
    if ((rv = configValue(cfg, section, "Bold", CFG_BOOL, &bold))) {
        LOG(L_WARN, "Could not read value Bold, using default = false");
        bold = 1;
    }

    int underline;
    if ((rv = configValue(cfg, section, "Underline", CFG_BOOL, &underline))) {
        LOG(L_WARN, "Could not read value Underline, using default = false");
        underline = 1;
    }

    int colorCode = 39;
    for (size_t i = 0; i != sizeof(table)/sizeof(*table); ++i) {
        if (!strcmp(color, table[i].name)) {
            colorCode = table[i].code;
            break;
        }
    }
    size_t maxPrefixLength = strlen("\x1B[1;4;xxm") + 1;
    char *prefix = (char *)malloc(maxPrefixLength);
    if (!prefix) {
        LOG(L_FATAL, "Allocation failed (%zu bytes)", maxPrefixLength);
        return -1;
    }
    sprintf(prefix, "\x1B[%s%s%dm",
            bold ? "1;" : "",
            underline ? "4;" : "",
            colorCode);
    
    decoration->prefix = prefix;
    decoration->prefixLength = strlen(decoration->prefix);
    decoration->suffix = "\x1B[0m";
    decoration->suffixLength = strlen(decoration->suffix);

    LOG(L_DEBUG, "decoration refix: %s", decoration->prefix);
    LOG(L_DEBUG, "decoration suffix: %s", decoration->suffix);
    return 0;
}

MODULE_PRIVATE
void responseCleanup(struct Query *query) {
    free(query->response);
    query->response = NULL;
    query->responseLength = 0;
}

MODULE_PRIVATE
void decorate(struct Module *module, struct Query *query, int postProcess) {
    LOG(L_DEBUG, "executed");
    struct Decoration *decoration = (struct Decoration *)module->privateData;
    if (!decoration) {
        query->responseCode = RC_ERROR;
        return;
    }

    const char *source = postProcess ?
        query->response :
        query->query;
    size_t length = postProcess ?
        query->responseLength :
        query->queryLength;

    size_t responseLength = decoration->prefixLength
                          + length
                          + decoration->suffixLength
                          + 1;
    char *output = (char *) malloc(responseLength);
    if (!output) {
        LOG(L_FATAL, "Allocation failed (%zu bytes)", responseLength);
        query->responseCode = RC_ERROR;
        return;
    }
    char *p = output;
    strcpy(p, decoration->prefix);
    p += decoration->prefixLength;
    strcpy(p, source);
    p += length;
    strcpy(p, decoration->suffix);

    if (query->responseCleanup)
        query->responseCleanup(query);
    query->response = output;
    query->responseLength = responseLength;
    query->responseCleanup = responseCleanup;

    LOG(L_DEBUG, "finite");

    query->responseCode = RC_SUCCESS;
}

MODULE_PRIVATE
void process(struct Module *module, struct Query *query) {
    decorate(module, query, 0);
}

MODULE_PRIVATE
void postProcess(struct Module *module, struct Query *query) {
    decorate(module, query, 1);
}

MODULE_PRIVATE
void cleanup(struct Module *module) {
    if (!module)
        return;
    struct Decoration *decoration = (struct Decoration *)module->privateData;
    if (!decoration)
        return;
    free((char *)decoration->prefix);
    free(decoration);
}


void moduleDecorate(struct Module *module) {
    memset(module, 0, sizeof(struct Module));
    struct Decoration *decoration = (struct Decoration *)malloc(sizeof(struct Decoration));
    if (!decoration) {
        LOG(L_FATAL, "Allocation failed (%zu bytes)", sizeof(struct Decoration));
        return;
    }


    module->privateData = decoration;
    module->name = "decorate";
    module->loadConfig = loadConfig;
    module->process = process;
    module->postProcess = postProcess;
    module->cleanup = cleanup;
}