#ifndef ADVICE_HANDLER_H
#define ADVICE_HANDLER_H

#include "http_router/http_router.h"
#include "http_router/query_parameters/query_parameters.h"

HTTP_Status_Code advice_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context);

#endif // ADVICE_HANDLER_H
