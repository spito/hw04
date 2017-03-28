#include "module-magic.h"

MODULE_PRIVATE
void process(struct module *module, struct query *query)
{
    (void)module;
    query->responseCode = RCSuccess;
}

void moduleMagic(struct module *module)
{
    module->privateData = NULL;
    module->name = "magic";
    module->loadConfig = NULL;
    module->process = process;
    module->postProcess = NULL;
    module->cleanup = NULL;
}