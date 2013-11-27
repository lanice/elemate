
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
    if( ea.getHandled() )
        return false;

    switch( ea.getEventType() )
    {
        // case osgGA::GUIEventAdapter::MOVE:
        //     return handleMouseMove( ea, us );

        // case osgGA::GUIEventAdapter::DRAG:
        //     return handleMouseDrag( ea, us );

        // case osgGA::GUIEventAdapter::PUSH:
        //     return handleMousePush( ea, us );

        // case osgGA::GUIEventAdapter::RELEASE:
        //     return handleMouseRelease( ea, us );

        case osgGA::GUIEventAdapter::KEYDOWN:
            return handleKeyDown( ea, us );

        // case osgGA::GUIEventAdapter::KEYUP:
        //     return handleKeyUp( ea, us );

        // case osgGA::GUIEventAdapter::SCROLL:
        //     if( _flags & PROCESS_MOUSE_WHEEL )
        //     return handleMouseWheel( ea, us );
        //     else
        //     return false;

        default:
            return false;
    }
}


bool GodManipulator::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    if( ea.getKey() == osgGA::GUIEventAdapter::KEY_F )
    {
        m_world->makeStandardBall();
        return true;
    }

    return false;
}
