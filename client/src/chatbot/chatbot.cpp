#include "chatbot.h"

#include "../server_info/server_info.h"

Chatbot::Chatbot()
{
    is_online = Server_Info::IsServerOnline("http://free-virus.com:1234");

    if (is_online)
    {
        selection_text = "Ask AI [ONLINE]";
        return;
    }
    selection_text = "Ask AI [OFFLINE]";
}


void Chatbot::Ask(Keyboard& keyboard)
{
    if (!this->is_online)
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

bool Chatbot::IsOnline()
{
    return this->is_online;
}

std::string& Chatbot::GetSelectionText()
{
    return this->selection_text;
}