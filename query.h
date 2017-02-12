#ifndef QUERY_H
#define QUERY_H

#include <stdint.h>

#include "functions.h"

enum responseCode {
    RCError,
    RCContinue,
    RCSuccess,
    RCNoResponse,
    RCInvalidInput
};

struct query {
    const char *query;
    size_t queryLength;
    queryCleanupFn queryCleanup;

    char *response;
    size_t responseLength;
    enum responseCode responseCode;
    queryCleanupFn responseCleanup;
};

void initQuery(struct query *);

#endif
