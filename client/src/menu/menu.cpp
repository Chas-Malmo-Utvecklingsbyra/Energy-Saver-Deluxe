#include "menu.h"

#include <unistd.h>
#include <iostream>
#include <thread>
#include <unistd.h>

Selection::Selection(std::string_view string, std::size_t index, std::function<void(void)> callback)
    : string(string), index(index), callback(callback)
{}

Selection::Selection(std::string_view string, std::size_t index, std::function<void(Keyboard&)> callback)
    : string(string), index(index), input_callback(callback)
{}

void Menu::Add_Selection(std::string_view selection, std::function<void(void)> callback)
{
    std::size_t selection_size = this->selections.size() + 1;
    std::size_t new_index = 0;


    new_index = selection_size - 1;
    Selection select(selection, new_index, callback);

    this->selections.emplace_back(select);

    if (this->selected == nullptr)
    {
        this->selected = &this->selections.at(0);
    }
}

void Menu::Add_Selection(std::string_view selection, std::function<void(Keyboard&)> callback)
{
    std::size_t selection_size = this->selections.size() + 1;
    std::size_t new_index = 0;


    new_index = selection_size - 1;
    Selection select(selection, new_index, callback);

    this->selections.emplace_back(select);

    if (this->selected == nullptr)
    {
        this->selected = &this->selections.at(0);
    }

}

std::vector<Selection>& Menu::Get_Selections()
{
    return this->selections;
}

Menu::Menu()
{
    this->selected = nullptr;
}

void Menu::Menu_Print()
{
    system("clear");
    std::cout << "\033[1;34mWIP Menu: (Press '0' to exit!)\033[0m\n";
    //this->selected.selection = selections.at(this->selected.index);
    //this->selected = &selections.at(this->selected->index + 1);

    for (auto &selection : selections)
    {
        if (selection.index == this->selected->index)
        {

            std::cout << "\033[1;32m" << selection.string << "\033[0m\n";
        }
        else
        {
            std::cout << selection.string << "\n";
        }
    }
}


void Menu::Select_Down()
{
    if (this->selected->index+1 >= selections.size())
        return;

    this->selected = &selections.at(this->selected->index + 1);
    Menu_Print();
}

void Menu::Select_Up()
{
    if (this->selected->index == 0)
        return;

    this->selected = &selections.at(this->selected->index - 1);
    Menu_Print();
}

Selection *Menu::Get_Selection()
{
    return this->selected;
}