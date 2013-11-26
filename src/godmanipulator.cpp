
#include "godmanipulator.h"


GodManipulator::GodManipulator()
   : inherited()
{
}


GodManipulator::GodManipulator( const GodManipulator& geh, const osg::CopyOp& copyOp )
   : Object(geh, copyOp),
     inherited( geh, copyOp )
{
}


bool GodManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    return false;
}