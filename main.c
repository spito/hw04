#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "log.h"
#include "config.h"
#include "module-cache.h"
#include "module-toupper.h"
#include "module-decorate.h"


void process(const char *queryText, struct Module *modules, int modulesCount) {

    struct Query query;
    memset(&query, 0, sizeof(struct Query));
    query.query = queryText;
    query.queryLength = strlen(queryText);
    query.queryCleanup = NULL;

    LOG(L_INFO, "query: %s", queryText);

    int m = 0;
    for (; m < modulesCount; ++m) {
        LOG(L_DEBUG, "Running module %s", modules[m].name);
        modules[m].process(&modules[m], &query);

        int leave = 0;
        switch (query.responseCode) {
        case RC_SUCCESS:
            LOG(L_INFO, "Response success");
            leave = 1;
            break;
        case RC_CONTINUE:
            LOG(L_INFO, "Response continue");
            continue;
        case RC_INVALID_INPUT:
            LOG(L_WARN, "Response invalid input");
            leave = 1;
            break;
        case RC_NO_RESPONSE:
            LOG(L_WARN, "No response");
            continue;
        case RC_ERROR:
        default:
            LOG(L_ERROR, "Error");
            leave = 1;
            break;
        }
        if (leave)
            break;
    }
    LOG(L_DEBUG, "responseCode: %i", query.responseCode);
    if (query.responseCode == RC_SUCCESS || query.responseCode == RC_NO_RESPONSE) {
        m = modulesCount;
        while (--m >= 0) {
            if (modules[m].postProcess) {
                LOG(L_DEBUG, "Postprocessing by %s", modules[m].name);
                modules[m].postProcess(&modules[m], &query);
            }
        }
    }
    LOG(L_INFO, "response: %s", query.response);
    printf("query: %s\nresponse: %s\n", query.query, query.response);

    if (query.responseCleanup)
        query.responseCleanup(&query);
    if (query.queryCleanup)
        query.queryCleanup(&query);
}

void setLogSetting(const struct Config *cfg) {
    const char *logFile;
    if (!configValue(cfg, "log", "File", CFG_STRING, &logFile)) {
        if (setLogFile(logFile))
            LOG(L_WARN, "Invalid value for File: '%s'", logFile);
    }

    const char *logMask;
    if (!configValue(cfg, "log", "Mask", CFG_STRING, &logMask)) {
        if (!strcmp(logMask, "D") || !strcmp(logMask, "d"))
            setLogMask(L_DEBUG);
        else if (!strcmp(logMask, "I") || !strcmp(logMask, "i"))
            setLogMask(L_INFO);
        else if (!strcmp(logMask, "W") || !strcmp(logMask, "w"))
            setLogMask(L_WARN);
        else if (!strcmp(logMask, "E") || !strcmp(logMask, "e"))
            setLogMask(L_ERROR);
        else if (!strcmp(logMask, "F") || !strcmp(logMask, "f"))
            setLogMask(L_FATAL);
        else if (!strcmp(logMask, "N") || !strcmp(logMask, "n"))
            setLogMask(L_NO_LOG);
        else
            LOG(L_WARN, "Invalid value for Mask: '%s'", logMask);
    }
}

int loadConfig(const char *configFile,
               struct Module *modules,
               int modulesCount) {
    struct Config cfg;
    int rv = configReader(&cfg, configFile);
    if (rv) {
        switch (rv) {
        case 1:
            LOG(L_ERROR, "Config file '%s' cannot be opened", configFile);
            break;
        case 2:
            LOG(L_ERROR, "Config file '%s' is corrupted", configFile);
            break;
        }
        return rv;
    }

    setLogSetting(&cfg);

    char section[265] = "plugin::";
    char *moduleName = section + strlen(section);
    for (int m = 0; m < modulesCount; ++m) {
        if (!modules[m].loadConfig)
            continue;

        strcpy(moduleName, modules[m].name);
        LOG(L_DEBUG, "Loading config of section '%s'", section);
        if ((rv = modules[m].loadConfig(&modules[m], &cfg, section))) {
            LOG(L_WARN, "Config loading failed (module: '%s', rv: %i)", modules[m].name, rv);
        }
    }
    return 0;
}

void processFile(const char *file,
                 struct Module *modules,
                 int modulesCount) {
    LOG(L_DEBUG, "Opening file '%s'", file);
    FILE *input = fopen(file, "r");
    if (!input) {
        LOG(L_ERROR, "Cannot open file '%s'", file);
        return;
    }
    char line[64 + 1] = {0};

    for (int i = 1; fgets(line, 64, input); ++i) {
        
        for (char *end = line + strlen(line) - 1; isspace(*end); --end) {
            *end = '\0';
        }

        LOG(L_DEBUG, "line: '%s'", line);
        process(line, modules, modulesCount);
    }
    fclose(input);
}

int main(int argc, char **argv) {
    setLogMask(L_DEBUG);
    setLogFile("server.log");
    LOG(L_INFO, "Start");

    if (argc < 2) {
        LOG(L_ERROR, "Cannot run without input file name");
        fprintf(stderr, "Cannot run without input file name\n");
        return 6;
    }

    const char *configFile = "server.conf";
    int rv;

    int modulesCount = 3;

    struct Module modules[3];
    moduleCache(&modules[0]);
    moduleToUpper(&modules[1]);
    moduleDecorate(&modules[2]);
    //moduleToLower(&modules[2]);

    if ((rv = loadConfig(configFile, modules, modulesCount))) {
        return rv;
    }

    processFile(argv[1], modules, modulesCount);

    for (int m = 0; m < modulesCount; ++m) {
        if (modules[m].cleanup)
            modules[m].cleanup(&modules[m]);
    }

    LOG(L_INFO, "Quit");
    return 0;
}