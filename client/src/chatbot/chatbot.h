#ifndef CHATBOT_H
#define CHATBOT_H

#include <iostream>
#include <cstring>

#include "../keyboard/keyboard.h"
extern "C"
{
    #include "http/client/httpClient.h"
    #include "weather/http.h"
    #include "json/cJSON/cJSON.h"
}

class Chatbot
{
private:
    bool is_online = false;
    std::string selection_text;

public:
    Chatbot();
    void Ask(Keyboard& keyboard);
    bool IsOnline();
    std::string& GetSelectionText();
};

#endif