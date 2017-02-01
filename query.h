#ifndef QUERY_H
#define QUERY_H

#include <stdint.h>

#include "functions.h"

enum ResponseCode {
    RC_ERROR,
    RC_CONTINUE,
    RC_SUCCESS,
    RC_NO_RESPONSE,
    RC_INVALID_INPUT
};

struct Query {

    const char *query;
    size_t queryLength;
    queryCleanupFn queryCleanup;

    char *response;
    size_t responseLength;
    enum ResponseCode responseCode;
    queryCleanupFn responseCleanup;


};

void initQuery(struct Query *);

#endif
