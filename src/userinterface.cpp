#include "userinterface.h"

UserInterface::UserInterface()
{
}

UserInterface::~UserInterface()
{
}

void UserInterface::initialize()
{
    m_stringDrawer.initialize();
}

void UserInterface::showHUD()
{
    m_stringDrawer.paint("It works", glm::mat4());
}

void UserInterface::showMainMenu()
{

}