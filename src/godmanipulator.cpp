
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
        case osgGA::GUIEventAdapter::MOVE:
            return handleMouseMove( ea, us );

        case osgGA::GUIEventAdapter::DRAG:
            return handleMouseDrag( ea, us );

        case osgGA::GUIEventAdapter::PUSH:
            return handleMousePush( ea, us );

        case osgGA::GUIEventAdapter::RELEASE:
            return handleMouseRelease( ea, us );

        case osgGA::GUIEventAdapter::KEYDOWN:
            return handleKeyDown( ea, us );

        case osgGA::GUIEventAdapter::KEYUP:
            return handleKeyUp( ea, us );

        case osgGA::GUIEventAdapter::SCROLL:
            return handleMouseWheel( ea, us );

        default:
            return false;
    }
}


bool GodManipulator::handleMouseMove( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


bool GodManipulator::handleMouseDrag( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


bool GodManipulator::handleMousePush( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


bool GodManipulator::handleMouseRelease( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


bool GodManipulator::handleKeyDown( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
	switch (ea.getKey()){
		case osgGA::GUIEventAdapter::KEY_F:
			m_world->makeStandardBall();
			return true;
			break;

		case osgGA::GUIEventAdapter::KEY_L:
			m_world->moveSoundRight(1);
			return true;
			break;

		case osgGA::GUIEventAdapter::KEY_J:
			m_world->moveSoundLeft(1);
			return true;
			break;

		case osgGA::GUIEventAdapter::KEY_I:
			m_world->moveSoundForw(1);
			return true;
			break;

		case osgGA::GUIEventAdapter::KEY_K:
			m_world->moveSoundBackw(1);
			return true;
			break;

		case osgGA::GUIEventAdapter::KEY_U:
			m_world->moveSoundUp(1);
			return true;
			break;

		case osgGA::GUIEventAdapter::KEY_O:
			m_world->moveSoundDown(1);
			return true;
			break;
	}

    return false;
}


bool GodManipulator::handleKeyUp( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


bool GodManipulator::handleMouseWheel( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}
