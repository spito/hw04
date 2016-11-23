#include <memory.h> 

#include "query.h"

void initQuery(struct Query *query) {
    memset(query, 0, sizeof(struct Query));
    query->responseCode = RC_NO_RESPONSE;
}
