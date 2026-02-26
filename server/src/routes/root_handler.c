#include "root_handler.h"
#include "logger/logger.h"

typedef struct HTTP_Cool_Context HTTP_Cool_Context;
struct HTTP_Cool_Context
{
    Logger *logger;
};

HTTP_Status_Code root_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context)
{
    (void)params;
    (void)response;
    (void)route_context;
    HTTP_Cool_Context *cool_context = (HTTP_Cool_Context *)registry_context;

    response->content_type = HTTP_CONTENT_TYPE_HTML;
    Logger_Write(cool_context->logger, "%s", "Hii");
    // TODO: Log here?

    return HTTP_STATUS_CODE_NOT_FOUND;
}