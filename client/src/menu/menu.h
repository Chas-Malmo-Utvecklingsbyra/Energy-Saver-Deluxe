#ifndef MENU_H
#define MENU_H

#include <vector>
#include <string>

class Selected
{
public:
    std::string_view selection;
    std::size_t index;
};

class Menu
{
private:
    std::vector<std::string_view> selections;
    Selected selected;

public:
    Menu();
    void Add_Selection(std::string_view selection);
    std::vector<std::string_view>& Get_Selections();
    void Menu_Print();
    void Select_Down();
    void Select_Up();
};

#endif