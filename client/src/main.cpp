#include <iostream>

#include "keyboard/keyboard.h"
#include "menu/menu.h"

extern "C"
{
    #include "http/client/httpClient.h"
    #include "weather/http.h"
    #include "json/cJSON/cJSON.h"
}

#define SERVER_ADDR "localhost"
#define TEST_PORT 8080

static void On_Received_Full_Message(HTTPClient *client){
    printf("%s", client->inbuffer);
}

void Get_Weather_Report_Data()
{
    HTTPClient client;
    
    if (HTTPClient_Initiate(&client, On_Received_Full_Message) != 0)
    {
        std::cout << "Failed to Initiate HTTPClient" << "\n";
        return;
    }

    std::cout << "Hello, this is the client :)\n";

    HTTPClient_GET(&client, SERVER_ADDR, "/advice", TEST_PORT);

    while (HTTPClient_Work(&client) == false);

    HTTPClient_Dispose(&client);
}

static bool AI_IsOffline = false;
void Ask_AI(Keyboard& keyboard)
{
    if (AI_IsOffline)
    {
        std::cout << "AI is currently offline, please try again later" << std::endl;
        return;
    }

    std::cout << "Input: ";
    keyboard.Toggle();

    std::string input;
    std::getline(std::cin, input);

    Http h = {0};

    Http_Error initialize_result = http_initialize(&h);
    if (initialize_result != HTTP_SUCCESSFUL)
    {
        std::cout << "Failed to Initialize!\n";
        return;
    }

    struct curl_slist *headers = nullptr;

    headers = curl_slist_append(headers, "Content-Type: application/json");


    std::string prompt = "Tell me the top trending model on hugging face";

    std::string data =
    R"({
        "model": "openai/gpt-oss-20b",
        "input": ")" + input + R"("
    })";
    
    char *response = nullptr;
    http_post(&h, "http://free-virus.com/api/v1/chat", (char*)data.c_str(), headers, &response);
    http_dispose(&h, nullptr);

    //std::cout << response << std::endl;

    cJSON *root = cJSON_Parse(response);
    cJSON *output = cJSON_GetObjectItem(root, "output");
    cJSON *output_msg = cJSON_GetArrayItem(output, 1);
    cJSON *content = cJSON_GetObjectItem(output_msg, "content");

    std::cout << cJSON_GetStringValue(content) << std::endl;

    cJSON_Delete(root);
    free(response);
    keyboard.Toggle();
}

bool isServerOnline(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);        // HEAD request
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);       // overall timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L); // connection timeout

    // Disable all retries
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L); // no redirect following
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 0L);      // max redirects = 0
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);    // fail on HTTP errors

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

int main()
{
    Keyboard keyboard;
    Menu menu;
    
    menu.Add_Selection("Get Weather Report Data", Get_Weather_Report_Data);

    if (isServerOnline("http://31.209.27.130:1234"))
    {
        menu.Add_Selection("Ask AI [ONLINE]", Ask_AI);
    }
    else
    {
        AI_IsOffline = true;
        menu.Add_Selection("Ask AI [OFFLINE]", Ask_AI);
    }


    menu.Menu_Print();

    keyboard.Read([&](Keyboard_Code key) 
    {
        //std::cout << "CHAR: " << ch << " | " << " CODE: " << (int)ch << "\n";

        if (key == Keyboard_Code::ENTER)
        {
            Selection *selected = menu.Get_Selection();

            if (selected->callback != nullptr)
                selected->callback();
            else if (selected->input_callback != nullptr)
                selected->input_callback(keyboard);
        }

        if (key == Keyboard_Code::ZERO)
        {
            std::cout << "Exiting!\n";
            return false;
        }

        if (key == Keyboard_Code::ARROW_UP)
        {
            menu.Select_Up();
        }

        if (key == Keyboard_Code::ARROW_DOWN)
        {
            menu.Select_Down();
        }

        return true;
    });

    return 0;
}
