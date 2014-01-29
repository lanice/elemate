#pragma once

#include "rendering/string_rendering/StringDrawer.h";

class UserInterface{
public:
    UserInterface();
    ~UserInterface();

    void initialize();
    void showHUD();
    void showMainMenu();

protected:
    StringDrawer m_stringDrawer;

private:
    UserInterface(const UserInterface&) = delete;
    void operator=(const UserInterface&) = delete;
};