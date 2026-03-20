#include "advice_handler.h"
#include "file_helper/file_helper.h"
#include <time.h>

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
    struct tm tomorrow = *localtime(&current_time);
    tomorrow.tm_mday += + 1;
    mktime(&tomorrow);

    char filename[64];
    snprintf(filename, sizeof(filename), "Energy_Advice_SE4_%04d-%02d-%02d.json", tomorrow.tm_year + 1900, tomorrow.tm_mon + 1, tomorrow.tm_mday);

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