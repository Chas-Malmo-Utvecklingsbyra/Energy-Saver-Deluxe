#ifndef SERVER_INFO_H
#define SERVER_INFO_H

#include <iostream>
#include <cstring>

extern "C"
{
    #include "http/client/httpClient.h"
    #include "weather/http.h"
    #include "json/cJSON/cJSON.h"
}

class Server_Info
{
public:
    static bool IsServerOnline(std::string_view url);
};

#endif