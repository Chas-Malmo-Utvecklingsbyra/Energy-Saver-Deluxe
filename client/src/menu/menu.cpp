#include "menu.h"

#include <unistd.h>
#include <iostream>
#include <thread>
#include <unistd.h>

void Menu::Add_Selection(std::string_view string)
{
    if (this->selected.selection == "" && this->selected.index == 0)
    {
        this->selected.selection = string;
    }

    this->selections.emplace_back(string);
}

std::vector<std::string_view>& Menu::Get_Selections()
{
    return this->selections;
}

Menu::Menu()
{
    this->selected.selection = "";
    this->selected.index = 0;
}

void Menu::Menu_Print()
{
    system("clear");
    std::cout << "\033[1;34mWIP Menu: (Press '0' to exit!)\033[0m\n";
    this->selected.selection = selections.at(this->selected.index);

    for (auto &selection : selections)
    {
        if (selection == this->selected.selection)
        {
            std::cout << "\033[1;32m" << selection << "\033[0m\n";
        }
        else
        {
            std::cout << selection << "\n";
        }
    }
}


void Menu::Select_Down()
{
    if (this->selected.index+1 >= selections.size())
        return;

    this->selected.index++;
    Menu_Print();
}

void Menu::Select_Up()
{
    if (this->selected.index == 0)
        return;

    this->selected.index--;
    Menu_Print();
}