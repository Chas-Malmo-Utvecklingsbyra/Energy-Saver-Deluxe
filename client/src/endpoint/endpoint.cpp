#include "endpoint.h"

#define SERVER_ADDR "localhost"
#define TEST_PORT 8080

static void On_Received_Full_Message(HTTPClient *client){
    printf("%s", client->inbuffer);
}

Endpoint::Endpoint(std::string_view endpoint)
{
    this->endpoint = endpoint;
}

void Endpoint::Get()
{
    HTTPClient client;
    
    if (HTTPClient_Initiate(&client, On_Received_Full_Message) != 0)
    {
        std::cout << "Failed to Initiate HTTPClient" << "\n";
        return;
    }

    std::cout << "Collecting " + this->endpoint +  " from Server...\n";

    HTTPClient_GET(&client, SERVER_ADDR, this->endpoint.c_str(), TEST_PORT);

    while (HTTPClient_Work(&client) == false);

    HTTPClient_Dispose(&client);
}