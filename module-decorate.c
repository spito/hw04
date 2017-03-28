#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
#include "log.h"
#include "module-decorate.h"

MODULE_PRIVATE const char *defaultPrefix = "\x1B[39m";

enum colors {
    ColorBlack = 30,
    ColorRed = 31,
    ColorGreen = 32,
    ColorYellow = 33,
    ColorBlue = 34,
    ColorMagneta = 35,
    ColorCyan = 36,
    ColorLightGray = 37,
    ColorDefault = 39
};

MODULE_PRIVATE
struct {
    const char *name;
    int code;
} table[] = {
    {"black", ColorBlack},
    {"red", ColorRed},
    {"green", ColorGreen},
    {"yellow", ColorYellow},
    {"blue", ColorBlue},
    {"magneta", ColorMagneta},
    {"cyan", ColorCyan},
    {"light gray", ColorLightGray},
    {"default", ColorDefault}
};

struct decoration {
    const char *prefix;
    size_t prefixLength;
    const char *suffix;
    size_t suffixLength;
};

MODULE_PRIVATE
int loadConfig(struct module *module, const struct config *cfg, const char *section)
{
    struct decoration *decoration = (struct decoration *)module->privateData;
    if (!decoration) {
        LOG(LError, "Corrupted module");
        return -1;
    }

    int rv;
    const char *color;
    if ((rv = configValue(cfg, section, "Color", CfgString, &color))) {
        LOG(LWarn, "Could not read value Color, using default = default");
        color = "default";
    }

    int bold;
    if ((rv = configValue(cfg, section, "Bold", CfgBool, &bold))) {
        LOG(LWarn, "Could not read value Bold, using default = false");
        bold = 0;
    }

    int underline;
    if ((rv = configValue(cfg, section, "Underline", CfgBool, &underline))) {
        LOG(LWarn, "Could not read value Underline, using default = false");
        underline = 0;
    }

    int colorCode = 0;
    for (size_t i = 0; i != sizeof(table)/sizeof(*table); ++i) {
        if (!strcmp(color, table[i].name)) {
            colorCode = table[i].code;
            break;
        }
    }
    if (!colorCode) {
        LOG(LWarn, "Requested color '%s' is not available, using color = default", color);
        colorCode = ColorDefault;
    }
    size_t maxPrefixLength = strlen("\x1B[1;4;xxm") + 1;
    char *prefix = (char *)malloc(maxPrefixLength);
    if (!prefix) {
        LOG(LFatal, "Allocation failed (%zu bytes)", maxPrefixLength);
        return -1;
    }
    sprintf(prefix, "\x1B[%s%s%dm",
            bold ? "1;" : "",
            underline ? "4;" : "",
            colorCode);

    if (decoration->prefix) {
        free((char *)decoration->prefix);
    }

    decoration->prefix = prefix;
    decoration->prefixLength = strlen(decoration->prefix);

    LOG(LDebug, "decoration refix: %s", decoration->prefix);
    return 0;
}

MODULE_PRIVATE
void responseCleanup(struct query *query)
{
    free(query->response);
    query->response = NULL;
    query->responseLength = 0;
}

MODULE_PRIVATE
void decorate(struct module *module, struct query *query, int postProcess)
{
    LOG(LDebug, "executed");
    struct decoration *decoration = (struct decoration *)module->privateData;
    if (!decoration || !decoration->prefix) {
        query->responseCode = RCError;
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
        LOG(LFatal, "Allocation failed (%zu bytes)", responseLength);
        query->responseCode = RCError;
        return;
    }
    char *p = output;
    strcpy(p, decoration->prefix);
    p += decoration->prefixLength;
    strcpy(p, source);
    p += length;
    strcpy(p, decoration->suffix);

    if (query->responseCleanup) {
        query->responseCleanup(query);
    }
    query->response = output;
    query->responseLength = responseLength;
    query->responseCleanup = responseCleanup;

    LOG(LDebug, "finite");

    query->responseCode = RCSuccess;
}

MODULE_PRIVATE
void process(struct module *module, struct query *query)
{
    decorate(module, query, 0);
}

MODULE_PRIVATE
void postProcess(struct module *module, struct query *query)
{
    decorate(module, query, 1);
}

MODULE_PRIVATE
void cleanup(struct module *module)
{
    if (!module) {
        return;
    }
    struct decoration *decoration = (struct decoration *)module->privateData;
    if (!decoration) {
        return;
    }
    free((char *)decoration->prefix);
    free(decoration);
}


void moduleDecorate(struct module *module)
{
    memset(module, 0, sizeof(struct module));
    struct decoration *decoration = (struct decoration *)malloc(sizeof(struct decoration));
    if (!decoration) {
        LOG(LFatal, "Allocation failed (%zu bytes)", sizeof(struct decoration));
        return;
    }

    module->privateData = decoration;
    module->name = "decorate";
    module->loadConfig = loadConfig;
    module->process = process;
    module->postProcess = postProcess;
    module->cleanup = cleanup;

    decoration->suffix = "\x1B[0m";
    decoration->suffixLength = strlen(decoration->suffix);
    decoration->prefixLength = strlen(defaultPrefix);
    char *prefix = (char *)malloc(decoration->prefixLength + 1);
    if (!prefix) {
        LOG(LFatal, "Allocation failed (%zu bytes)", sizeof(struct decoration));
        return;
    }
    strcpy(prefix, defaultPrefix);
    decoration->prefix = prefix;

}