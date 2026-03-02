#include <iostream>

#include "keyboard/keyboard.h"
#include "menu/menu.h"

void hello_print()
{
    std::cout << "print hello!\r\n";
}


int main() {
    Keyboard keyboard;
    Menu menu;
    menu.Add_Selection("Hello World!");
    menu.Add_Selection("Johnny!", hello_print);

    menu.Menu_Print();

    keyboard.Read([&](Keyboard_Code ch) 
    {
        //std::cout << "CHAR: " << ch << " | " << " CODE: " << (int)ch << "\n";

        if (ch == Keyboard_Code::ENTER)
        {
            Selection *selected = menu.Get_Selection();

            if (selected->callback != nullptr)
                selected->callback();
        }

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
}