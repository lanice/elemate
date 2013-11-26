
#include "godeventhandler.h"


GodEventHandler::GodEventHandler()
   : inherited()
{
}


GodEventHandler::GodEventHandler( const GodEventHandler& geh, const osg::CopyOp& copyOp )
   : Object(geh, copyOp),
     inherited( geh, copyOp )
{
}


bool GodEventHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    return false;
}