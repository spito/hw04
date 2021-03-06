#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "log.h"
#include "config.h"
#include "module-cache.h"
#include "module-toupper.h"
#include "module-tolower.h"
#include "module-decorate.h"
#include "module-magic.h"


void process(const char *queryText, struct module *modules, int modulesCount) {

    struct query query;
    memset(&query, 0, sizeof(struct query));
    query.query = queryText;
    query.queryLength = strlen(queryText);
    query.queryCleanup = NULL;
    query.response = "";
    query.responseCleanup = NULL;

    LOG(LInfo, "query: %s", queryText);

    int m = 0;
    for (; m < modulesCount; ++m) {
        LOG(LDebug, "Running module %s", modules[m].name);
        modules[m].process(&modules[m], &query);

        switch (query.responseCode) {
        case RCSuccess:
            LOG(LInfo, "Response success");
            continue;
        case RCDone:
            LOG(LInfo, "Response done");
            break;
        case RCError:
        default:
            LOG(LError, "Error");
            break;
        }
        break;
    }
    LOG(LDebug, "responseCode: %i", query.responseCode);
    if (query.responseCode == RCSuccess) {
        m = modulesCount;
        while (--m >= 0) {
            if (modules[m].postProcess) {
                LOG(LDebug, "Postprocessing by %s", modules[m].name);
                modules[m].postProcess(&modules[m], &query);
            }
        }
    }
    LOG(LInfo, "response: %s", query.response);
    printf("query: %s\nresponse: %s\n", query.query, query.response);

    if (query.responseCleanup) {
        query.responseCleanup(&query);
    }
    if (query.queryCleanup) {
        query.queryCleanup(&query);
    }
}

void setLogSetting(const struct config *cfg)
{
    const char *logFile;
    if (!configValue(cfg, "log", "File", CfgString, &logFile)) {
        if (setLogFile(logFile)) {
            LOG(LWarn, "Invalid value for File: '%s'", logFile);
        }
    }

    const char *logMask;
    if (!configValue(cfg, "log", "Mask", CfgString, &logMask)) {
        if (!strcmp(logMask, "D") || !strcmp(logMask, "d")) {
            setLogMask(LDebug);
        } else if (!strcmp(logMask, "I") || !strcmp(logMask, "i")) {
            setLogMask(LInfo);
        } else if (!strcmp(logMask, "W") || !strcmp(logMask, "w")) {
            setLogMask(LWarn);
        } else if (!strcmp(logMask, "E") || !strcmp(logMask, "e")) {
            setLogMask(LError);
        } else if (!strcmp(logMask, "F") || !strcmp(logMask, "f")) {
            setLogMask(LFatal);
        } else if (!strcmp(logMask, "N") || !strcmp(logMask, "n")) {
            setLogMask(LNoLog);
        } else {
            LOG(LWarn, "Invalid value for Mask: '%s'", logMask);
        }
    }
}

int loadConfig(const char *configFile,
               struct module *modules,
               int modulesCount)
{
    struct config cfg;
    int rv = configRead(&cfg, configFile);
    switch (rv) {
    case 0:
        break;
    case 1:
        LOG(LError, "Config file '%s' cannot be opened", configFile);
        break;
    case 2:
        LOG(LError, "Config file '%s' is corrupted", configFile);
        break;
    }

    setLogSetting(&cfg);

    char section[265] = "plugin::";
    char *moduleName = section + strlen(section);
    for (int m = 0; m < modulesCount; ++m) {
        if (!modules[m].loadConfig) {
            continue;
        }
        strcpy(moduleName, modules[m].name);
        LOG(LDebug, "Loading config of section '%s'", section);
        if ((rv = modules[m].loadConfig(&modules[m], &cfg, section))) {
            LOG(LWarn, "Config loading failed (module: '%s', rv: %i)", modules[m].name, rv);
        }
    }
    configClean(&cfg);
    return 0;
}

void processFile(const char *file,
                 struct module *modules,
                 int modulesCount)
{
    LOG(LDebug, "Opening file '%s'", file);
    FILE *input = fopen(file, "r");
    if (!input) {
        LOG(LError, "Cannot open file '%s'", file);
        return;
    }
    char line[64 + 1] = {0};

    for (int i = 1; fgets(line, 64, input); ++i) {
        
        for (char *end = line + strlen(line) - 1; isspace(*end); --end) {
            *end = '\0';
        }

        LOG(LDebug, "line: '%s'", line);
        process(line, modules, modulesCount);
    }
    fclose(input);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        LOG(LError, "Cannot run without input file name");
        return 6;
    }

    const char *configFile = "server.conf";

    int modulesCount = 5;

    struct module modules[5];
    moduleCache(&modules[0]);
    moduleToUpper(&modules[1]);
    moduleDecorate(&modules[2]);
    moduleToLower(&modules[3]);
    moduleMagic(&modules[4]);

    int rv;
    if ((rv = loadConfig(configFile, modules, modulesCount))) {
        if (rv == 1)
            LOG(LWarn, "config file %s is missing", configFile);
        else
            return rv;
    }
    LOG(LInfo, "Start");

    struct module selectedModules[] = {
        modules[0],
        modules[1],
        modules[2]
    };
    int selectedModulesCount = 3;

    processFile(argv[1], selectedModules, selectedModulesCount);

    for (int m = 0; m < modulesCount; ++m) {
        if (modules[m].cleanup) {
            modules[m].cleanup(&modules[m]);
        }
    }

    LOG(LInfo, "Finished");
    return 0;
}