#include <stdio.h>

#include "http/server/http_server.h"

HTTP_Status_Code route_root(QueryParameters_t *params, Request_Handler_Response_t *response, void *context)
{   
    (void)params;
    (void)response;
    (void)context;

    response->content_type = HTTP_CONTENT_TYPE_HTML;

    return HTTP_STATUS_CODE_NOT_FOUND;
}

int main()
{
    printf("Hello, Energy Saver Deluxe!\n");
    uint16_t port = 8080;
    size_t max_connections = 1024;

    HTTP_Server http_server;

    if(HTTP_Server_Initialize(&http_server, max_connections) == false)
    {
        printf("Server failed to initialize\n");
        return -1;
    }

    HTTP_Server_Register_Route(&http_server, "/", route_root);

    if(HTTP_Server_Start(&http_server, port) == false)
    {
        printf("Server failed to start\n");
        return -2;
    }

    while(1)
    {
        HTTP_Server_Work(&http_server);
    }

    HTTP_Server_Dispose(&http_server);    
    
    return 0;
}