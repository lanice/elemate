
#include "godmanipulator.h"


GodManipulator::GodManipulator()
   : inherited()
{
}


GodManipulator::GodManipulator( const GodManipulator& gm, const osg::CopyOp& copyOp )
   : Object(gm, copyOp),
     inherited( gm, copyOp )
{
}


bool GodManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    return false;
}