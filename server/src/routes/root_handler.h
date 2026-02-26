#ifndef ROOT_HANDLE_H
#define ROOT_HANDLE_H

#include "http_router/http_router.h"
#include "http_router/query_parameters/query_parameters.h"

HTTP_Status_Code root_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context);


#endif // ROOT_HANDLE_H