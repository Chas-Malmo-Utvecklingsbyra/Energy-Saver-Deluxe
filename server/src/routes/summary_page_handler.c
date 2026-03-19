#include "summary_page_handler.h"
#include "json/fileHelper/fileHelper.h"
#include "file_helper/file_helper.h"

HTTP_Status_Code summary_page_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context)
{
    (void)params;
    (void)route_context;
    (void)registry_context;

    char *html = file_read_to_string("./frontend/summary.html");
    if (!html)
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;

    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, html, false);

    return response->status_code;
}