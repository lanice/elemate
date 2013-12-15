#include <iostream>

#include "game.h"

#include <osgViewer/Viewer>
int main()
{
    osgViewer::Viewer viewer;
    // setup view and graphics context
    viewer.setUpViewInWindow(50, 50, 500, 500);
    //viewer.setUpViewOnSingleScreen();

    Game game(viewer);

    game.start();

    return 0;
}
