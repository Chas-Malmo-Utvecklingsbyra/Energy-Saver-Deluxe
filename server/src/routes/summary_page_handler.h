#ifndef SUMMARY_PAGE_HANDLER_H
#define SUMMARY_PAGE_HANDLER_H

#include "http_router/http_router.h"
#include "http_router/query_parameters/query_parameters.h"

HTTP_Status_Code summary_page_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context);

#endif // SUMMARY_PAGE_HANDLER_H
