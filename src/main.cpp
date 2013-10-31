#include <osgViewer/Viewer>
#include <osg/GraphicsContext>
#include <iostream>
#include "worlddrawable.h"

int main( int argc, char** argv )
{
    osgViewer::Viewer viewer;
    osg::ref_ptr<osg::Geode> root = new osg::Geode();
    WorldDrawable * world = new WorldDrawable;

    // root gets ownership of the world, so do not delete thy
    root->addDrawable(world);
    viewer.setSceneData(root.get());

    // use the OpenGL state im needed..
    // osg::StateSet * stateSet = root->getOrCreateStateSet();

    return viewer.run();
}
