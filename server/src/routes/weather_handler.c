#include "weather_handler.h"
#include "file_helper/file_helper.h"

HTTP_Status_Code weather_handler_handle(QueryParameters_t *params, Route_Handler_Response_t *response, void *route_context, void *registry_context)
{
    (void)params;
    // (void)response;
    (void)route_context;
    (void)registry_context;
    printf("Weather handler called!\n");

    char* response_data;
    size_t response_size = 0;

    File_Helper_Result result = File_Helper_Read("./data/weather", "weather_SE4.json", &response_data, &response_size);
    if (FILE_HELPER_RESULT_SUCCESS != result)
    {
        printf("Error code: %d\r\n", result);
        return HTTP_STATUS_CODE_INTERNAL_SERVER_ERROR;
    }

    Http_Router_Set_Response(response, HTTP_STATUS_CODE_OK, HTTP_CONTENT_TYPE_JSON, response_data, false);

    return response->status_code;
}