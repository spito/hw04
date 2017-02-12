#include "config.h"

int configReader(struct config *cfg, const char *name)
{
    /// TODO: implement
    return 0;
}


int configValue(const struct config *cfg,
                const char *section,
                const char *key,
                enum configValueType type,
                void *value)
{
    /// TODO: implement
    return 2;
}


void configCleaner(struct config *cfg)
{
    /// TODO: implement
}
