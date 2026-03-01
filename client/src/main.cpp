#include <iostream>

#include "keyboard/keyboard.h"

int main() {
    Keyboard keyboard;

    std::cout << "WIP Menu: (Press '0' to exit!)\n";

    keyboard.Read([](Keyboard_Code ch) 
    {
        //std::cout << "CHAR: " << ch << " | " << " CODE: " << (int)ch << "\n";

        if (ch == Keyboard_Code::ZERO)
        {
            std::cout << "Exiting!\n";
            return false;
        }

        return true;
    });

    return 0;
}