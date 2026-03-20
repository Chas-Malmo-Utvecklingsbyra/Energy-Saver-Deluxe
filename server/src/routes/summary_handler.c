#include "summary_handler.h"
#include "file_helper/file_helper.h"
#include <time.h>

HTTP_Status_Code summary_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context)
{
    (void)params;
    // (void)response;
    (void)route_context;
    (void)registry_context;
    printf("Summary handler called!\n");

    char* response_data;
    size_t response_size = 0;

    time_t current_time = time(NULL);
    struct tm tomorrow = *localtime(&current_time);
    tomorrow.tm_mday += + 1;
    mktime(&tomorrow);

    char filename[64];
    snprintf(filename, sizeof(filename), "Energy_Advice_Summary_SE4_%04d-%02d-%02d.txt", tomorrow.tm_year + 1900, tomorrow.tm_mon + 1, tomorrow.tm_mday);

    File_Helper_Result result = File_Helper_Read("./Energy_Advice_Report_Summary", filename, &response_data, &response_size);
    if (FILE_HELPER_RESULT_SUCCESS != result)
    {
        printf("Error code: %d\r\n", result);
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    }

    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_HTML, response_data, false);

    return response->status_code;
}