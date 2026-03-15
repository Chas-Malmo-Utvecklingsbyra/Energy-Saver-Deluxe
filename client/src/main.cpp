#include <iostream>
#include <cstring>

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

static std::string http_response;

static void On_Received_Full_Message(HTTPClient *client){
    printf("%s", client->inbuffer);
}

static void On_Received_Summary(HTTPClient *client)
{
    http_response = (char*)client->inbuffer;
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

void Get_Energy_Summary()
{
    HTTPClient client;

    if (HTTPClient_Initiate(&client, On_Received_Summary) != 0)
    {
        std::cout << "Failed to initiate HTTP Client\n";
        return;
    }

    HTTPClient_GET(&client, SERVER_ADDR, "/advice", TEST_PORT);

    while (HTTPClient_Work(&client) == false);

    char *json_start = strstr((char*)http_response.c_str(), "\r\n\r\n");
    if (!json_start)
    {
        std::cout << "Invalid HTTP Response\n";
        HTTPClient_Dispose(&client);
        return;
    }

    json_start += 4;

    cJSON *energy = cJSON_Parse(json_start);
    if (!energy)
    {
        std::cout << "Failed to parse JSON\n";
        HTTPClient_Dispose(&client);        
        return;
    }        

    cJSON *summary = cJSON_GetObjectItem(energy, "summary");
    if (!summary)
    {
        std::cout << "Summary does not exist!\n";
        cJSON_Delete(energy);
        HTTPClient_Dispose(&client);        
        return;
    }

    std::cout << "\n====================== SUMMARY FOR THE DAY ======================\n\n";
    cJSON *charge = cJSON_GetObjectItem(summary, "charge");
    cJSON *found_charge = cJSON_GetObjectItem(charge, "found");
    if (!cJSON_IsTrue(found_charge))
    {
        std::cout << "The best time to CHARGE from grid: There is no window that fulfills the requirements today\n";
    }
    else
    {
        std::cout << "The best time to CHARGE from grid: " 
            << cJSON_GetObjectItem(charge, "start")->valuestring 
            << " - "
            << cJSON_GetObjectItem(charge, "end")->valuestring
            << " (avg "
            << cJSON_GetObjectItem(charge, "avg")->valuedouble
            << ")\n";
    }
    
    cJSON *consume = cJSON_GetObjectItem(summary, "consume");
    cJSON *found_consume = cJSON_GetObjectItem(consume, "found");
    if (!cJSON_IsTrue(found_consume))
    {
        std::cout << "The best time to CONSUME solar: There is no window that fulfills the requirements today\n";
    }
    else
    {
        std::cout << "The best time to CONSUME solar: "
            << cJSON_GetObjectItem(consume, "start")->valuestring
            << " - "
            << cJSON_GetObjectItem(consume, "end")->valuestring
            << " (avg "
            << cJSON_GetObjectItem(consume, "avg")->valuedouble
            << ")\n";
    }

    cJSON *sell = cJSON_GetObjectItem(summary, "sell");
    cJSON *found_sell = cJSON_GetObjectItem(sell, "found");
    if (!cJSON_IsTrue(found_sell))
    {
        std::cout << "The best time to SELL energy: There is no window that fulfills the requirements today\n";
    }
    else
    {
        std::cout << "The best time to SELL energy: "
            << cJSON_GetObjectItem(sell, "start")->valuestring
            << " - "
            << cJSON_GetObjectItem(sell, "end")->valuestring
            << " (avg "
            << cJSON_GetObjectItem(sell, "avg")->valuedouble
            << ")\n";
    }

    std::cout << "\n=================================================================\n";

    cJSON_Delete(energy);
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
    menu.Add_Selection("Get Energy Summary", Get_Energy_Summary);

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
            std::vector<Selection>& selection_arr = menu.Get_Selections();
            Selection& selected = selection_arr.at(menu.Get_SelectedIndex());

            if (selected.callback != nullptr)
                selected.callback();
            else if (selected.input_callback != nullptr)
                selected.input_callback(keyboard);
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
