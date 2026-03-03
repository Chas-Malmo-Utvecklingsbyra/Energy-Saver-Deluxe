#include "root_handler.h"
#include "logger/logger.h"
#include "file_helper/file_helper.h"
#include "json/fileHelper/fileHelper.h"
#include <time.h>

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

    char buffer[2046];
    memset(buffer, 0, sizeof(buffer));

    char* html = file_read_to_string("./frontend/index.html");

    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, html, true);

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

    time_t current_time = time(NULL);
    struct tm *time = localtime(&current_time);

    char filename[64]; 
    // TODO PL: Still has to be changed, because the data should cover the next coming day, but right now the json-file is the current date, so it works for now.
    snprintf(filename, sizeof(filename), "Energy_Advice_%04d-%02d-%02d.json", time->tm_year + 1900, time->tm_mon + 1, time->tm_mday);
    
    File_Helper_Result result = File_Helper_Read("./Energy_Advice_Reports", filename, &response_data, &response_size);
    if (FILE_HELPER_RESULT_SUCCESS != result)
    {
        printf("Error code: %d\r\n", result);
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    }

    char final_response[response_size];

    snprintf(final_response, response_size, "%s", response_data);

    free(response_data);
    response_data = NULL;

    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_JSON, final_response, false);

    //Logger_Write(cool_context->logger, "%s", "Write stuff here I guess"); fix this, logger should not be null
    return response->status_code;
}