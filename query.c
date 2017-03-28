#include <memory.h> 

#include "query.h"

void initQuery(struct query *query)
{
    memset(query, 0, sizeof(struct query));
    query->responseCode = RCSuccess;
}
