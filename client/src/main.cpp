#include <iostream>
#include "keyboard/keyboard.h"
#include "menu/menu.h"

extern "C"
{
    #include "http/client/httpClient.h"
}

#define SERVER_ADDR "localhost:8080"
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

    HTTPClient_GET(&client, SERVER_ADDR, "/advice");

    while (HTTPClient_Work(&client) == false);

    HTTPClient_Dispose(&client);
}

void Ask_AI()
{
    std::cout << "Coming soon\n";
}

int main()
{
    Keyboard keyboard;
    Menu menu;
    
    menu.Add_Selection("Get Weather Report Data", Get_Weather_Report_Data);
    menu.Add_Selection("Ask AI [Coming Soon]", Ask_AI);

    menu.Menu_Print();

    keyboard.Read([&](Keyboard_Code key) 
    {
        //std::cout << "CHAR: " << ch << " | " << " CODE: " << (int)ch << "\n";

        if (key == Keyboard_Code::ENTER)
        {
            Selection *selected = menu.Get_Selection();

            if (selected->callback != nullptr)
                selected->callback();
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
