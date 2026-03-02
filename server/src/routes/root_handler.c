#include "root_handler.h"
#include "logger/logger.h"
#include "file_helper/file_helper.h"

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

HTTP_Status_Code advice_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context)
{
    (void)params;
    // (void)response;
    (void)route_context;
    
    HTTP_Cool_Context *cool_context = (HTTP_Cool_Context *)registry_context;

    char* response_data; // Energy_Advice json files are about 31000 characters
    size_t response_size = 0;

    File_Helper_Read("../../../../Energy_Advice_Reports", "Energy_Advice_2026-03-02.json", &response_data, &response_size);
    
    printf("[[[FILE_SIZE: %ld]]]", response_size);

    char final_response[response_size];

    snprintf(final_response, response_size, "%s", response_data);

    free(response_data);
    response_data = NULL;

    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, final_response);

    Logger_Write(cool_context->logger, "%s", "Write stuff here I guess");

    return response->status_code;
}