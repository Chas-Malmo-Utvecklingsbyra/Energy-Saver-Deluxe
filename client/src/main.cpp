#include "chatbot/chatbot.h"
#include "endpoint/endpoint.h"

#include "keyboard/keyboard.h"
#include "menu/menu.h"

int main()
{
    Keyboard keyboard;
    Menu menu;

    Endpoint weather_endpoint("/weather");
    menu.Add_Selection("Get Weather Report Data", [&weather_endpoint]{ weather_endpoint.Get(); });

    Endpoint advice_endpoint("/advice");
    menu.Add_Selection("Get Spot Price Report Data", [&advice_endpoint]{ advice_endpoint.Get(); });


    Endpoint summary_endpoint("/summary");
    menu.Add_Selection("Get Energy Summary", [&summary_endpoint]{ summary_endpoint.Get(); });

    Chatbot chatbot;
    menu.Add_Selection(chatbot.GetSelectionText(), [&chatbot, &keyboard]
    {
        chatbot.Ask(keyboard);
    });

    menu.Menu_Print();

    keyboard.Read([&](Keyboard_Code key) 
    {
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
