#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <iostream>
#include <cstring>

extern "C"
{
    #include "http/client/httpClient.h"
    #include "weather/http.h"
    #include "json/cJSON/cJSON.h"
}

class Endpoint
{
    std::string endpoint;

public:
    Endpoint(std::string_view endpoint);
    void Get();

};

#endif