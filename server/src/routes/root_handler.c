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
    (void)registry_context; //temp
    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, "<h1>Hello, World!</h1>");

    response->content_type = HTTP_CONTENT_TYPE_HTML;
    // Logger_Write(cool_context->logger, "%s", "Hii"); fix this, logger should not be null

    return response->status_code;
}

//TODO fix errors
HTTP_Status_Code advice_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context)
{
    (void)params;
    // (void)response;
    (void)route_context;
    (void)registry_context;
    printf("Advice handler called!\n");

    char* response_data; // Energy_Advice json files are about 31000 characters
    size_t response_size = 0;

    File_Helper_Result result = File_Helper_Read("./Energy_Advice_Reports", "Energy_Advice_2026-03-02.json", &response_data, &response_size);
    if (FILE_HELPER_RESULT_SUCCESS != result)
    {
        printf("RESULT WAS NOT CORRECT\r\n");
        printf("Error code: %d\r\n", result);
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    }


    printf("[[[FILE_SIZE: %ld]]]", response_size);

    char final_response[response_size];

    snprintf(final_response, response_size, "%s", response_data);

    free(response_data);
    response_data = NULL;

    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, final_response);

    //Logger_Write(cool_context->logger, "%s", "Write stuff here I guess"); fix this, logger should not be null
    return response->status_code;
}