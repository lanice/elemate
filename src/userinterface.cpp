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
    m_stringDrawer.paint("Active element: water", glm::mat4(0.5, 0, 0, 0, 
                                                            0, 0.5, 0, 0, 
                                                            0, 0, 0.5, 0, 
                                                            -0.95, -0.95, 0, 1), StringDrawer::Alignment::kAlignLeft);
}

void UserInterface::showMainMenu()
{

}