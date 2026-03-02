#include <iostream>

extern "C"
{
    #include "http/client/httpClient.h"
}

#define SERVER_ADDR "localhost:8080"
#define TEST_PORT 8080

static void On_Received_Full_Message(HTTPClient *client){
    printf("%s", client->inbuffer);
}

int main()
{
    HTTPClient client;
    
    if (HTTPClient_Initiate(&client, On_Received_Full_Message) != 0)
        return -1;

    std::cout << "Hello, this is the client :)\n";

    HTTPClient_GET(&client, SERVER_ADDR, "/advice");

    while (HTTPClient_Work(&client) == false);

    HTTPClient_Dispose(&client);

    return 0;
}

/* #include <iostream>

#include "keyboard/keyboard.h"
#include "menu/menu.h"


int main() {
    Keyboard keyboard;
    Menu menu;
    menu.Add_Selection("Hello World!");
    menu.Add_Selection("Johnny!");

    menu.Menu_Print();


    keyboard.Read([&](Keyboard_Code ch) 
    {
        //std::cout << "CHAR: " << ch << " | " << " CODE: " << (int)ch << "\n";

        if (ch == Keyboard_Code::ZERO)
        {
            std::cout << "Exiting!\n";
            return false;
        }

        if (ch == Keyboard_Code::ARROW_UP)
        {
            menu.Select_Up();
        }

        if (ch == Keyboard_Code::ARROW_DOWN)
        {
            menu.Select_Down();
        }

        return true;
    });

    return 0;
} */