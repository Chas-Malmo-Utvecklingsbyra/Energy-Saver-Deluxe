#include <iostream>

#include "keyboard/keyboard.h"

int main() {
    Keyboard keyboard;

    std::cout << "WIP Menu: (Press '1' to exit!)\n";

    keyboard.Read([](char ch) 
    {
        std::cout << "CHAR: " << ch << " | " << " CODE: " << (int)ch << "\n";

        if (ch == '1')
        {
            return false;
        }

        return true;
    });

    return 0;
}