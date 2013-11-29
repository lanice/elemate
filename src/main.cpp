#include <iostream>
#include <thread>

#include "game.h"

#include <osgViewer/Viewer>
int main()
{
    osgViewer::Viewer* viewer = new osgViewer::Viewer();
	viewer->setUpViewInWindow(50, 50, 500, 500);

	Game game(*viewer);	

	game.start();
	game.end();

	return 0;
}
