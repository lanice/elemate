
#include "godmanipulator.h"

#include "world.h"


GodManipulator::GodManipulator()
   : inherited(),
     m_world(nullptr)
{
}


GodManipulator::GodManipulator( const GodManipulator& gm, const osg::CopyOp& copyOp )
   : Object(gm, copyOp),
     inherited( gm, copyOp ),
     m_world( gm.m_world )
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
	switch (ea.getUnmodifiedKey()){
		case osgGA::GUIEventAdapter::KEY_L:
        {
			m_world->moveSoundRight(1);
			m_world->moveSoundRight(2);
			return true;
			break;
        }
		case osgGA::GUIEventAdapter::KEY_J:
		{
			m_world->moveSoundLeft(1);
			m_world->moveSoundLeft(2);
			return true;
			break;
		}
		case osgGA::GUIEventAdapter::KEY_I:
        {
			m_world->moveSoundForw(1);
			m_world->moveSoundForw(2);
			return true;
			break;
        }
		case osgGA::GUIEventAdapter::KEY_K:
        {
			m_world->moveSoundBackw(1);
			m_world->moveSoundBackw(2);
			return true;
			break;
        }
		case osgGA::GUIEventAdapter::KEY_U:
        {
			m_world->moveSoundUp(1);
			m_world->moveSoundUp(2);
			return true;
			break;
        }
		case osgGA::GUIEventAdapter::KEY_O:
        {
			m_world->moveSoundDown(1);
			m_world->moveSoundDown(2);
			return true;
			break;
        }
		case osgGA::GUIEventAdapter::KEY_M:
        {
			m_world->doBomb(2);
			return true;
			break;
        }
        case osgGA::GUIEventAdapter::KEY_F:
        {
            m_world->makeStandardBall();
            return true;
        }
        case osgGA::GUIEventAdapter::KEY_F5:
        {
            m_world->reloadShader();
            return true;
        }
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
