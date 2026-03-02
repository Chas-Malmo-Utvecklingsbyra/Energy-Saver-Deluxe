#ifndef MENU_H
#define MENU_H

#include <vector>
#include <string>
#include <functional>

class Selection
{
public:
    std::string_view string;
    std::size_t index;
    std::function<void(void)> callback;

    Selection(std::string_view string, std::size_t index, std::function<void(void)> callback);
};

class Menu
{
private:
    std::vector<Selection> selections;
    Selection *selected;

public:
    Menu();
    void Add_Selection(std::string_view selection, std::function<void(void)> callback = nullptr);
    std::vector<Selection>& Get_Selections();
    void Menu_Print();
    void Select_Down();
    void Select_Up();
    Selection *Get_Selection();
};

#endif