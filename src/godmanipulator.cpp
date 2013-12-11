
#include "godmanipulator.h"

#include <cassert>

#include "world.h"
#include "godnavigation.h"
#include "terrain/terraininteractor.h"


GodManipulator::GodManipulator()
   : inherited(),
     m_world( nullptr ),
     _keyPressedAlt_L(false)
{
}


GodManipulator::GodManipulator( const GodManipulator& gm, const osg::CopyOp& copyOp )
   : Object(gm, copyOp),
     inherited( gm, copyOp ),
     m_world( gm.m_world ),
     _keyPressedAlt_L(gm._keyPressedAlt_L)
{
}

void GodManipulator::setNavigation( GodNavigation * navigation)
{
    m_navigation = navigation;
}

void GodManipulator::setWorld(std::shared_ptr<World>& world)
{
    m_world = world;
    m_terrainInteractor = std::make_shared<TerrainInteractor>(world->terrain);
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
        case osgGA::GUIEventAdapter::KEY_P:
        {
            m_world->toogleBackgroundSound(0);
            return true;
        }
        case osgGA::GUIEventAdapter::KEY_O:
        {
            m_world->toogleBackgroundSound(1);
            return true;
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
        case osgGA::GUIEventAdapter::KEY_Alt_L:
        {
            _keyPressedAlt_L = true;
            return true;
        }
    }
    return false;
}


bool GodManipulator::handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    switch (ea.getUnmodifiedKey()){
    case osgGA::GUIEventAdapter::KEY_Alt_L:
        _keyPressedAlt_L = false;
        return true;
    }
    return false;
}


bool GodManipulator::handleMouseWheel( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    if (_keyPressedAlt_L) {
        assert(m_navigation.valid());
        assert(m_terrainInteractor);

        osg::Vec3d position = m_navigation->getCenter();

        switch (ea.getScrollingMotion())
        {
        case osgGA::GUIEventAdapter::SCROLL_UP:
            m_terrainInteractor->changeHeight(position.x(), position.z(), TerrainLevel::BaseLevel, 1);
            return true;

        case osgGA::GUIEventAdapter::SCROLL_DOWN:
            m_terrainInteractor->changeHeight(position.x(), position.z(), TerrainLevel::BaseLevel, -1);
            return true;

        default:
            return false;
        }
    }
    return false;
}
