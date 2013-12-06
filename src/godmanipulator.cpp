
#include "godmanipulator.h"

#include <osg/MatrixTransform>

#include "world.h"
#include "hand.h"


GodManipulator::GodManipulator()
   : inherited(),
     m_world(nullptr),
     m_hand(new Hand())
{
}


GodManipulator::GodManipulator( const GodManipulator& gm, const osg::CopyOp& copyOp )
   : Object(gm, copyOp),
     inherited( gm, copyOp ),
     m_world( gm.m_world ),
     m_hand( gm.m_hand )
{
}


GodManipulator::~GodManipulator()
{
    delete m_hand;
}


void GodManipulator::setWorld( std::shared_ptr<World> world )
{
    m_world = world;
    m_world->root()->addChild( m_hand->transform() );
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
    switch (ea.getUnmodifiedKey()) {
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
