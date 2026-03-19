#include "root_handler.h"
#include "logger/logger.h"
#include "file_helper/file_helper.h"
#include "json/fileHelper/fileHelper.h"

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
    (void)registry_context; //temp

    char* html = file_read_to_string("./frontend/index.html");

    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, html, true);

    response->content_type = HTTP_CONTENT_TYPE_HTML;

    return response->status_code;
}