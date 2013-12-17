
#include "godmanipulator.h"

#include <cassert>

#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/Texture2D>
#include <osg/Image>

#include "world.h"
#include "godnavigation.h"
#include "terrain/elemateterrain.h"
#include "hand.h"


GodManipulator::GodManipulator()
   : inherited(),
     m_world(nullptr),
     m_navigation(nullptr),
     m_camera(nullptr),
     m_hand(new Hand()),
     _windowX(0),
     _windowY(0)
{
}


GodManipulator::GodManipulator( const GodManipulator& gm, const osg::CopyOp& copyOp )
   : Object(gm, copyOp),
     inherited( gm, copyOp ),
     m_world( gm.m_world ),
     m_navigation( gm.m_navigation ),
     m_camera( gm.m_camera ),
     m_hand( gm.m_hand ),
     _windowX( gm._windowX ),
     _windowY( gm._windowY )
{
}


GodManipulator::~GodManipulator()
{
    delete m_hand;
}


void GodManipulator::setWorld( std::shared_ptr<World> world )
{
    m_world = world;
    m_hand->transform()->getOrCreateStateSet()->setAttribute(m_world->programByName("hand"));
    m_world->root()->addChild( m_hand->transform() );
}


void GodManipulator::setNavigation( GodNavigation * navigation )
{
    m_navigation = navigation;
}


void GodManipulator::setCamera( osg::Camera * camera )
{
    m_camera = camera;
    _windowX = camera->getViewport()->width();
    _windowY = camera->getViewport()->height();
}


bool GodManipulator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    if( ea.getHandled() )
        return false;

    switch( ea.getEventType() )
    {
        case osgGA::GUIEventAdapter::FRAME:
            return handleFrame( ea, us );

        case osgGA::GUIEventAdapter::RESIZE:
            return handleResize( ea, us );

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


bool GodManipulator::handleFrame( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    updateHandPosition( ea );
    return false;
}


bool GodManipulator::handleResize( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    _windowX = ea.getWindowWidth();
    _windowY = ea.getWindowHeight();
    return false;
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
            if (!isFountainOn){
                m_world->startFountainSound();
                isFountainOn = true;
            }
            m_world->updateFountainPosition();
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


bool GodManipulator::handleKeyUp( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*us*/ )
{
    switch (ea.getUnmodifiedKey()){
    case osgGA::GUIEventAdapter::KEY_F:
    {
        m_world->endFountainSound();
        isFountainOn = false;
        return true;
    }
    }
    return false;
}


bool GodManipulator::handleMouseWheel( const osgGA::GUIEventAdapter& /*ea*/, osgGA::GUIActionAdapter& /*us*/ )
{
    return false;
}


void GodManipulator::updateHandPosition( const osgGA::GUIEventAdapter& ea )
{
    osg::Vec3d eye, center, up;
    m_navigation->getTransformation( eye, center, up );

    osg::Matrixd matrix = osg::Matrixd::inverse( m_camera->getViewMatrix() * m_camera->getProjectionMatrix() );

    float x = ( ea.getX() / _windowX - 0.5 ) * 2.;
    float y = ( ea.getY() / _windowY - 0.5 ) * 2.;

    osg::Vec3 lookAtView = osg::Vec3( x, y, 1. ) * matrix;

    osg::Vec3 pos = eye - ( lookAtView * eye.y()/lookAtView.y() );
    pos.y() = m_world->terrain->heightAt( pos.x(), pos.z() ) + 1.;

    osg::Vec3d homeEye, homeCenter, homeUp;
    m_navigation->getHomePosition( homeEye, homeCenter, homeUp );

    osg::Vec3 from = homeEye-homeCenter; from.y() = 0.;
    osg::Vec3 to = eye-center; to.y() = 0.;

    osg::Matrixd handMatrix = osg::Matrixd::rotate( from, to );

    m_hand->transform()->setMatrix( m_hand->defaultTransform() * handMatrix * osg::Matrixd::translate( pos ) );
}
