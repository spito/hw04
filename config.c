#include "config.h"

int configReader(struct Config *cfg, const char *name) {
    /// TODO: implement
    return 0;
}


int configValue(const struct Config *cfg,
                const char *section,
                const char *key,
                enum ConfigValueType type,
                void *value)
{
    /// TODO: implement
    return 2;
}


void configCleaner(struct Config *cfg) {
    /// TODO: implement
}
